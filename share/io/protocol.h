#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdio.h>
#include <set>
#include <deque>
#include <algorithm>
#include <functional>

#include "passiveio.h"
#include "activeio.h"
#include "timeactiveio.h"
#include "marshal.h"
#include "thread.h"
#include "timer.h"
#include "conf.h"
#include "log.h"
#include "map.h"
#include "statistic.h"

namespace GNET
{

class Protocol : public Marshal
{
	friend class ProtocolBinder;
	friend class CompressBinder;

	void * _context; // transient
public:
	class Exception { };
	typedef unsigned int Type;

	static int typeHigh(Type type) { return type >> 16; }
	static int typeLow(Type type)  { return type & 0x0000ffff; }

	void * getContext()            { return _context; }
	void setContext(void * c)      { _context = c; }

protected:
	Type type;
	Protocol() { _context = NULL; }
private:
	struct Represent
	{
		struct Rep
		{
			Octets rep;
			Thread::Mutex locker;
			Octets Encode(const Protocol *protocol)
			{
				Thread::Mutex::Scoped l(locker);
				if ( rep.size() == 0 )
				{
					Marshal::OctetsStream ps;
					protocol->Encode(ps);
					rep.swap(ps);
				}
				return rep;
			}
			Octets Encode(      Protocol *protocol)
			{
				Marshal::OctetsStream ps;
				protocol->Encode(ps);
				Thread::Mutex::Scoped l(locker);
				if ( rep.size() == 0 || rep != ps )
					rep.swap(ps);
				return rep;
			}
			Rep() : locker("Protocol::Represent::Rep::locker") { }
			Rep(const Rep& rhs) : rep(rhs.rep), locker("Protocol::Represent::Rep::locker") { }
		};
		typedef std::map<const Protocol *, Rep> Map;
		static Map *map;
		static Thread::Mutex locker;
		static void Release(const Protocol *protocol)
		{
			Thread::Mutex::Scoped l(locker);
			map->erase(protocol);
		}
		static Rep& GetRep(const Protocol *protocol)
		{
			Thread::Mutex::Scoped l(locker);
			Map::iterator it = map->find(protocol);
			if ( it == map->end() ) it = map->insert(std::make_pair(protocol, Rep())).first;
			return (*it).second;
		}
		static Octets Encode(const Protocol *protocol) { return GetRep(protocol).Encode(protocol); }
		static Octets Encode(      Protocol *protocol) { return GetRep(protocol).Encode(protocol); }
	};
	typedef gnet_map<Type, const Protocol*> Map;
	static Map& GetMap() { static Map map; return map; }
protected:
	virtual ~Protocol() { Represent::Release(this); }
	Protocol(Type t) : type(t)
	{
		_context = NULL;
		if (!GetStub(type)) GetMap().insert(std::make_pair(type, this));
	}
	Protocol(const Protocol &rhs) : type(rhs.type) { _context = NULL; }
public:
	void Encode(Marshal::OctetsStream& os) const
	{
		Marshal::OctetsStream data;
		data << *this;
		if ( SizePolicy(data.size()) )
			os << CompactUINT(type) << data;
		else
			Log::log(LOG_ERR,"FATAL, Protocol Size Excceed(type=%d,size=%d).", type, data.size());
	}
	Octets Encode() const { return Represent::Encode(this); }
	Octets Encode()       { return Represent::Encode(this); }
	static const Protocol *GetStub(Type type)
	{
		Map::const_iterator it = GetMap().find(type);
		return it == GetMap().end() ? NULL : (*it).second;
	}
	static Protocol *Create(Type type)
	{
		const Protocol *stub = GetStub(type);
		return stub ? stub->Clone() : NULL;
	}
	class Manager
	{
	public:
		class Session : public NetSession
		{
			friend class Protocol;
			friend class Manager;
			friend class ProtocolBinder;
			friend class CompressBinder;
		public:
			class State
			{
				typedef std::set<Type> Set;
				Set set;
				int timeout;
			public:
				State (Type *first, size_t len, int t = 0) : set(first, first+len), timeout(t) { }
				State (Type *first, Type *last, int t = 0) : set(first, last), timeout(t) { }
				bool TypePolicy(Type type) const { return set.find(type) != set.end(); }
				bool TimePolicy(int t) const { return timeout <= 0 || t < timeout; }
				size_t SetSize() const { return set.size(); }
			};

			typedef unsigned int ID;
			size_t GetOsbytes() { return os_bytes; }
			
		protected:
			struct Stream : public Marshal::OctetsStream
			{
				Session *session;
				mutable bool check_policy;
				mutable bool proto_ignore;
				Stream(Session *s) : session(s), check_policy(true), proto_ignore(false) { }
			};
			typedef gnet_map<ID, Session *> Map;
			static ID session_id;
			static Thread::Mutex session_id_mutex;
			ID nextsid() { Thread::Mutex::Scoped l(session_id_mutex); return ++session_id; }

			ID sid;
			Stream is;
			std::deque<Octets>os;
			size_t os_bytes;
			Manager *manager;
			const State *state;
			Timer timer;
			int verbose;
			bool ignore;

			NetSession *Clone () const	{ return new Session(*this); }

			Session(Manager *m) : sid(nextsid()), is(this), os_bytes(0), manager(m), state(m->GetInitState()), verbose(LOG_ERR),ignore(false){ }
			Session(const Session &rhs) : NetSession(rhs), sid(nextsid()), is(this), os_bytes(rhs.os_bytes), manager(rhs.manager),
						      state(rhs.state), verbose(rhs.verbose),ignore(false){ }
		private:
			std::string Identification() const { return manager->Identification(); }
			void OnCheckAddress(SockAddr &sa) const { manager->OnCheckAddress(sa); }
			void OnOpen(const SockAddr& local, const SockAddr& peer)
			{
				timer.Reset();
				manager->AddSession(sid, this);
				manager->OnSetTransport(sid, local, peer);
			}
			void OnOpen()  { timer.Reset(); manager->AddSession(sid, this); }
			void OnClose() { manager->DelSession(sid, GetStatus()); }
			void OnAbort(const SockAddr &sa) { manager->AbortSession(sa, this); }
			void OnRecv()
			{
				Octets& input = Input();
				timer.Reset();
				is.insert(is.end(), input.begin(), input.end());
				input.clear();
				try
				{
					size_t count = 0;
					for (Protocol *p; (p = Protocol::Decode(is)); Protocol::Task::Dispatch(manager, sid, p))
					{
						count ++;
					}
					STAT_MIN5("Protocols",count);
				}
				catch (Marshal::Exception &) // marshal ! used wide
				{
					Close(CLOSE_EXCEPTION,false);
				}
				catch (Protocol::Exception &)
				{
					Close(CLOSE_ONERROR,false);
				}
			}
			void OnSend()
			{
				timer.Reset();
				if ( urgency_support )
				{
					if ( os_bytes && Output(os.front()) )
					{
						os_bytes -= os.front().size();
						os.pop_front();
					}
				}
				else
					for (;os_bytes && Output(os.front()); os_bytes -= os.front().size(), os.pop_front());
			}
			bool NoMoreData() const
			{
				return os_bytes == 0;
			}
			void OnCheckTimeout()
			{
				if (!state->TimePolicy(timer.Elapse()))
					Close(CLOSE_TIMEOUT,false);
			}
			bool Send(Octets ps, bool urg)
			{
				Thread::Mutex::Scoped l(locker);
				if (ps.size() && manager->OnCheckAccumulate(os_bytes))
				{
					os_bytes += ps.size();
					if ( urgency_support && urg )
						os.push_front(ps);
					else
						os.push_back(ps);
					SendReady();
					return true;
				}
				return false;
			}
			bool Send(const Protocol *protocol, bool urg) { return Send(protocol->Encode(), urg); }
			bool Send(      Protocol *protocol, bool urg) { return Send(protocol->Encode(), urg); }
			bool StatePolicy(Type type) const { return state->TypePolicy(type); }
			size_t AcceptSize() const { return state->SetSize(); }
			int  GetVerbose() const { return verbose; }
			void SetVerbose(int v) { verbose = v; }
			ID GetSid() const { return sid; }
			void ChangeState(const State *newstate)
			{
				Thread::Mutex::Scoped l(locker);
				timer.Reset();
				state = newstate;
			}
			void SetIgnore(bool val) { ignore = val;}
			bool Ignore() const { return ignore; }
		};
	private:
		Session::Map map;
		Thread::RWLock locker_map;
		void AddSession(Session::ID sid, Session *session)
		{
			{
			Thread::RWLock::WRScoped l(locker_map);
			map.insert(std::make_pair(sid, session));
			}
			OnAddSession(sid);
		}
		void DelSession(Session::ID sid, int status)
		{
			OnDelSession(sid, status);
			{
			Thread::RWLock::WRScoped l(locker_map);
			map.erase(sid);
			}
		}
		void AbortSession(const SockAddr &sa, Session *)
		{
			OnAbortSession(0);
			OnAbortSession(sa);
		}
		Session *GetSession(Session::ID sid)
		{
			Session::Map::iterator it = map.find(sid);
			return it == map.end() ? NULL : (*it).second;
		}
	public:
		Manager() : locker_map("Protocol::Manager::locker_map") { }
		virtual ~Manager() { }
		bool SetISecurity(Session::ID id, Security::Type type, const Octets &key)
		{
			Thread::RWLock::RDScoped l(locker_map);
			Session *session = GetSession(id);
			if (session) session->SetISecurity(type, key);
			return session != NULL;
		}
		bool SetOSecurity(Session::ID id, Security::Type type, const Octets &key)
		{
			Thread::RWLock::RDScoped l(locker_map);
			Session *session = GetSession(id);
			if (session) session->SetOSecurity(type, key);
			return session != NULL;
		}
		

#ifdef PROTOCOL_RELAXED
		HardReference<const Protocol>	ref_last;
		Octets rep;
		bool Send(Session::ID id, const Protocol *protocol, bool urg = false)
		{
			if ( OnSend(id, protocol) )
			{
				if( !rep || ref_last.GetObject() != protocol )
				{
					ref_last = HardReference<const Protocol>(protocol,NullDeleter());
					rep = protocol->Encode();
				}

				bool rc = false;
				Thread::RWLock::RDScoped l(locker_map);
				Session *session = GetSession(id);
				if (session) rc = session->Send(rep, urg);
				return rc;
			}
			return true;
		}
#else
		bool Send(Session::ID id, const Protocol *protocol, bool urg = false)
		{
			if ( OnSend(id, protocol) )
			{
				bool rc = false;
				Thread::RWLock::RDScoped l(locker_map);
				Session *session = GetSession(id);
				if (session) rc = session->Send(protocol, urg);
				return rc;
			}
			return true;
		}
#endif
		bool Send(Session::ID id,       Protocol *protocol, bool urg = false)
		{
			if ( OnSend(id, protocol) )
			{
				bool rc = false;
				Thread::RWLock::RDScoped l(locker_map);
				Session *session = GetSession(id);
				if (session) rc = session->Send(protocol, urg);
				return rc;
			}
			return true;
		}
		bool Send(Session::ID id, const Protocol &protocol, bool urg = false) { return Send(id,&protocol,urg); }
		bool Send(Session::ID id,       Protocol &protocol, bool urg = false) { return Send(id,&protocol,urg); }
		bool Close(Session::ID id, int status=CLOSE_ACTIVE)
		{
			Thread::RWLock::RDScoped l(locker_map);
			Session *session = GetSession(id);
			// close twice . is safe?
			if (session) 
				session->Close(status, false);
			return session != NULL;
		}
		bool ChangeState(Session::ID id, Session::State *state)
		{
			Thread::RWLock::RDScoped l(locker_map);
			Session *session = GetSession(id);
			if (session) session->ChangeState(state);
			return session != NULL;
		}
		virtual void OnAddSession(Session::ID) = 0;
		virtual void OnDelSession(Session::ID) = 0;
		virtual void OnDelSession(Session::ID sid, int status) {  OnDelSession(sid); }
		virtual void OnAbortSession(Session::ID) { }
		virtual void OnAbortSession(const SockAddr&) { }
		virtual bool OnCheckAccumulate(size_t) const { return true; }
		virtual void OnSetTransport(Session::ID sid, const SockAddr& local, const SockAddr& peer) { }
		virtual bool OnSend(Session::ID id, const Protocol *protocol) { return true; }
		virtual const Session::State *GetInitState() const = 0;
		virtual void CheckSessionTimePolicy()
		{
			Thread::RWLock::RDScoped l(locker_map);
			for (Session::Map::const_iterator it = map.begin(), ie = map.end(); it != ie; ++it)
				(*it).second->OnCheckTimeout();
		}
		virtual int  PriorPolicy(Protocol::Type type) const
		{
			return Protocol::GetStub(type)->PriorPolicy();
		}
		virtual bool InputPolicy(Protocol::Type type, size_t size) const
		{
			return Protocol::GetStub(type)->SizePolicy(size);
		}
		virtual std::string Identification() const = 0;
		virtual void OnCheckAddress(SockAddr &) const { }
		void SetVerbose(Session::ID sid, int v)
		{
			Thread::RWLock::WRScoped l(locker_map);
			Session *session = GetSession(sid);
			if(session)
				session->SetVerbose(v);
		}
		size_t GetOsbytes(Session::ID sid)
		{
			Thread::RWLock::WRScoped l(locker_map);
			Session *session = GetSession(sid);
			return session?session->GetOsbytes():0;
		}
		bool SetIgnore(Session::ID id, bool val)
		{
			Thread::RWLock::RDScoped l(locker_map);
			Session *session = GetSession(id);
			if (session) session->SetIgnore(val);
			return session != NULL;
		}
		 
		virtual bool CheckUnknownProtocol(Session::ID , Protocol::Type , unsigned int ) { return false; }
		virtual void DispatchUnknownProtocol(Session::ID , Protocol::Type , const OctetsStream & ) { }
	};
private:
	class Task : public Thread::Runnable
	{
		Manager *manager;
		Manager::Session::ID sid;
		Protocol *protocol;
	public:
		~Task () { protocol->Destroy(); }
		Task(Manager *m, Manager::Session::ID id, Protocol *p) : 
			manager(m), sid(id), protocol(p) { }
		void Run()
		{
			try
			{
				protocol->Process(manager, sid);
			}
			catch (Marshal::Exception &) // marshal ! used wide
			{
				manager->Close(sid);
			}
			catch (Protocol::Exception &)
			{
				manager->Close(sid);
			}
			delete this;
		}
		static void Dispatch(Manager *manager, Manager::Session::ID sid, Protocol *protocol)
		{
			Task *task = new Task(manager, sid, protocol);
			int priority = manager->PriorPolicy(protocol->type);
			if (priority)
			{
				task->SetPriority(priority);
				Thread::Pool::AddTask(task);
			}
			else
			{
				task->Run();
			}
		}
	};
private:
	static void CheckPolicy(Type type, unsigned int size, const Manager::Session::Stream &is)
	{
		if (Protocol::GetStub(type))
		{
			// origin check
			if (!is.session->StatePolicy(type))
			{
				if (!is.session->Ignore()) 
				{
					Log::log(is.session->GetVerbose(), "Protocol state error. sid=%d,type=%d,size=%d,acceptsize=%d",
							is.session->GetSid(),type,size,is.session->AcceptSize());
					throw Protocol::Exception();
				}
				else
				{
					is.proto_ignore = true;
				}
			}
			if (!is.session->manager->InputPolicy(type, size))
			{
				Log::log(is.session->GetVerbose(), "Protocol size policy error. sid=%d,type=%d,size=%d,acceptsize=%d",
						is.session->GetSid(),type,size,is.session->AcceptSize());
				throw Protocol::Exception();
			}
		}
		else
		{
			// unkown check
			if (!is.session->manager->CheckUnknownProtocol(is.session->GetSid(), type, size))
			{
				Log::log(is.session->GetVerbose(), "Protocol Unknown . sid=%d,type=%d,size=%d,acceptsize=%d",
						is.session->GetSid(),type,size,is.session->AcceptSize());
				throw Protocol::Exception();
			}
		}
		is.check_policy = false;
	}

	static Protocol *Decode(const Manager::Session::Stream &is)
	{
		Protocol *protocol = NULL;
		Protocol::Type type = 0;
		unsigned int size = 0;
		try
		{
			bool loop = true;
			while(loop)
			{
				if (is.eos()) return NULL;

				if (is.check_policy)
				{
					is >> Marshal::Begin >> CompactUINT(type) >> CompactUINT(size) >> Marshal::Rollback;
					CheckPolicy(type, size, is);
				}

				Manager::Session::Stream data(is.session);
				data.reserve(size);
				is >> Marshal::Begin >> CompactUINT(type) >> data >> Marshal::Commit;

				if (!is.proto_ignore)
				{
					if ((protocol = Protocol::Create(type))) 
					{
						data >> *protocol;
						loop = false;
					}
					else
					{
						is.session->manager->DispatchUnknownProtocol(is.session->GetSid(), type, data);
					}
				}
				is.check_policy = true;
				is.proto_ignore = false;
			}
		}
		catch (Marshal::Exception &)
		{
			if (protocol)
			{
				Log::log(LOG_DEBUG,"Protocol(type=%d,size=%d) Decode Error.", type, size);
				protocol->Destroy();
				throw Protocol::Exception();
			}
			is >> Marshal::Rollback;
		}
		return protocol;
	}
public:
	virtual void Process(Manager *, Manager::Session::ID) = 0;
	virtual Protocol *Clone() const = 0;
	virtual void Destroy() { delete this; }
	virtual int  PriorPolicy() const { return 0; }
	virtual bool SizePolicy(size_t) const { return true; }

	Type GetType() const { return type; }
	static PassiveIO *Server(Manager *manager) { return PassiveIO::Open(Manager::Session(manager)); }
	static PassiveIO *Server(Manager *manager, variant conf) { return PassiveIO::Open(Manager::Session(manager), conf); }
	static ActiveIO  *Client(Manager *manager) { return  ActiveIO::Open(Manager::Session(manager)); }
	static ActiveIO  *Client(Manager *manager, variant conf) { return  ActiveIO::Open(Manager::Session(manager), conf); }
	static TimeActiveIO *Client(Manager *manager, int timo) { return TimeActiveIO::Open(Manager::Session(manager), timo); }
	static TimeActiveIO *Client(Manager *manager, variant conf, int timo) { return TimeActiveIO::Open(Manager::Session(manager), conf, timo); }


	class AutoPtr
	{
		Protocol * p;

		// avoid copy
		AutoPtr(const AutoPtr &);	
		void operator = (const AutoPtr &);	
	public:
		explicit AutoPtr(Protocol * _p) : p(_p) { }
		~AutoPtr()                              { if (p) p->Destroy(); }
		Protocol * operator -> ()               { return p; }
		Protocol * get()                        { return p; }
		void release()                          { p = NULL; }
	};
};
};

#endif
