#include <assert.h>
#include <octets.h>
#include "conf.h"
#include "thread.h"
#include "gamedbclient.hpp"
#include "getrole.hrp"
#include "putrole.hrp"
#include "getmoneyinventory.hrp"
#include "putmoneyinventory.hrp"
#include "gpet"
#include "gpetcorral"
#include "gshoplog"
#include "gforcedatalist"
#include "db_if.h"
#include "glog.h"
#include "putspouse.hrp"
#include "putserverdata.hrp"
#include "getserverdata.hrp"
#include "getcashtotal.hrp"
#include "gconsumptionrecord"
#include "gmeridiandata"
#include "grolestatusextraprop"
#include "greincarnationdata"
#include "dbcopyrole.hrp"

namespace GDB
{
	namespace
	{
		inline void push_back(GNET::Octets & os, const ivec & vec)
		{
			if(vec.size) os.insert(os.end(),vec.data,vec.size);
		}
		inline void get_buf(const GNET::Octets & os, ivec & vec)
		{
			if((vec.size = os.size()))
			{
				vec.data = os.begin();
			}
			else
			{
				vec.data = NULL;
			}
		}

		inline void get_petcorral(const GPetCorral& tmpcor, petcorral& corral)
		{
			assert(corral.list == NULL && corral.count == 0);
			corral.capacity = tmpcor.capacity;
			corral.count = tmpcor.pets.size();
			corral.list = NULL;
			if(corral.count==0)
				return;
			corral.list = (pet*) malloc(sizeof(pet) * corral.count);
			pet* ptmp = corral.list;
			for(size_t i=0; i<corral.count; i++,ptmp++)
			{
				const GPet& tmppet = tmpcor.pets[i]; 
				ptmp->index = tmppet.index;
				ptmp->data.data = tmppet.data.begin();
				ptmp->data.size = tmppet.data.size();
			}
		}

		inline void set_petcorral(GNET::Octets& data, const petcorral& corral)
		{
			GPet tmppet;
			GPetCorral tmpcor;
			Marshal::OctetsStream os;

			tmpcor.capacity = corral.capacity; 
			const pet* ptmp = corral.list;
			for(size_t i=0;i<corral.count;i++,ptmp++)
			{
				tmppet.index = ptmp->index;
				tmppet.data.replace(ptmp->data.data, ptmp->data.size);
				tmpcor.pets.push_back(tmppet);
			}
			os << tmpcor;
			data = os;
		}

		inline void set_loglist(int roleid, std::vector<GShopLog>& glog, const loglist & list)
		{
			if(list.count == 0) return;
			const shoplog* p = list.list;
			glog.reserve(list.count);
			for(size_t i = 0; i < list.count; i ++,p++ )
			{
				GShopLog obj(roleid,p->order_id,p->item_id,p->expire,p->item_count,
						p->order_count,p->cash_need,p->time,p->guid1,p->guid2);
				glog.push_back(obj);
			}
		}


		inline 	void release_petcorral(petcorral& corral)
		{
			if(corral.count == 0) return;
			free(corral.list);
			corral.list = NULL;
			corral.count = 0;
		}

		inline void get_itemlist(const GNET::GRoleInventoryVector & backpack, itemlist & list)
		{
			assert(list.list == NULL && list.count == 0);
			list.count = backpack.size();
			list.list = NULL;
			if(list.count == 0) return;
			list.list = (itemdata*) malloc(sizeof(itemdata) * list.count);
			itemdata * pItem = list.list;
			for(size_t i = 0; i < list.count; i ++,pItem ++ )
			{
				const GRoleInventory &obj = backpack[i]; 
				pItem->id 	= obj.id;
				pItem->index 	= obj.pos;
				pItem->count 	= obj.count;
				pItem->max_count= obj.max_count;
				pItem->guid1 	= obj.guid1;
				pItem->guid2 	= obj.guid2;
				pItem->mask 	= obj.mask;
				pItem->proctype = obj.proctype;
				pItem->expire_date = obj.expire_date;
				pItem->data	= obj.data.begin();
				pItem->size	= obj.data.size();
			}
		}

		inline void set_itemlist(GNET::GRoleInventoryVector & backpack, const itemlist & list)
		{
			if(list.count == 0) return;
			const itemdata * pItem = list.list;
			backpack.GetVector().reserve(list.count);
			for(size_t i = 0; i < list.count; i ++,pItem ++ )
			{
				GRoleInventory  obj(pItem->id,pItem->index,pItem->count, pItem->max_count);
				obj.guid1 	= pItem->guid1;	
				obj.guid2 	= pItem->guid2; 	
				obj.mask 	= pItem->mask; 	
				obj.proctype 	= pItem->proctype; 
				obj.expire_date = pItem->expire_date;
				if(pItem->size) obj.data.replace(pItem->data,pItem->size);
				backpack.push_back(obj);
			}
		}

		inline 	void release_itemlist(itemlist & list)
		{
			if(list.count == 0) return;
			free(list.list);
			list.list = NULL;
			list.count = 0;
		}
		
		inline void get_factionrelation(const GFactionAllianceVector & alliance, const GFactionHostileVector & hostile, faction_relation_data & data)
		{
			data.alliance_count = alliance.size();
			data.alliance_list = NULL;
			if(data.alliance_count)
			{
				data.alliance_list = (int *)malloc(sizeof(int)*data.alliance_count);
				for(size_t i=0; i<data.alliance_count; i++)
					data.alliance_list[i] = alliance[i].fid;
			}
			data.hostile_count = hostile.size();
			data.hostile_list = NULL;
			if(data.hostile_count)
			{
				data.hostile_list = (int *)malloc(sizeof(int)*data.hostile_count);
				for(size_t i=0; i<data.hostile_count; i++)
					data.hostile_list[i] = hostile[i].fid;
			}
		}
		
		inline void release_factionrelation(faction_relation_data & data)
		{
			if(data.alliance_count)
			{
				free(data.alliance_list);
				data.alliance_list = NULL;
				data.alliance_count = 0;
			}
			if(data.hostile_count)
			{
				free(data.hostile_list);
				data.hostile_list = NULL;
				data.hostile_count = 0;
			}
		}

		inline void set_forcedata(GNET::Octets& force, const GDB::forcedata_list& list)
		{
			GNET::GForceDataList tmpforce;
			tmpforce.cur_force_id = list.cur_force_id;
			const GDB::forcedata * pData = list.list;
			for(size_t i=0; i<list.count; i++)
			{
				GNET::GForceData tmpdata;
				tmpdata.force_id = pData[i].force_id;
				tmpdata.reputation = pData[i].reputation;
				tmpdata.contribution = pData[i].contribution;
				tmpforce.list.push_back(tmpdata);
			}
			force = Marshal::OctetsStream()<<tmpforce;
		}
		inline void get_forcedata(const GNET::Octets & odata, GDB::forcedata_list& list)
		{
			GForceDataList tmpforce;
			try{
				if(odata.size())
				{
					Marshal::OctetsStream(odata) >> tmpforce;
				}
			}
			catch(Marshal::Exception)
			{
				tmpforce.cur_force_id = 0;
				tmpforce.list.clear();
			}
			
			list.cur_force_id = tmpforce.cur_force_id;
			list.count = tmpforce.list.size();
			list.list = NULL;
			if(list.count == 0) return;
			list.list = (GDB::forcedata*)malloc(sizeof(GDB::forcedata) * list.count);
			GDB::forcedata * pData = list.list;
			for(size_t i=0; i<list.count; i++)
			{
				const GNET::GForceData & tmpdata = tmpforce.list[i];
				pData[i].force_id = tmpdata.force_id;
				pData[i].reputation = tmpdata.reputation;
				pData[i].contribution = tmpdata.contribution;
			}
		}
		inline void release_forcedata(GDB::forcedata_list& list)
		{
			if(list.count)
			{
				free(list.list);
				list.count = 0;
				list.list = NULL;
			}
		}
		inline void set_countrydata(GNET::Octets& data, int country_id, int country_expire_time)
		{
			if(country_id)
			{
				data = Marshal::OctetsStream() << country_id << country_expire_time;
			}
		}
		inline void get_countrydata(const GNET::Octets& data, int & country_id, int & country_expire_time)
		{
			if(data.size())
			{
				try{
					Marshal::OctetsStream(data) >> country_id >> country_expire_time;
				}catch(Marshal::Exception)
				{
					country_id = country_expire_time = 0;
				}
			}
			else
			{
				country_id = country_expire_time = 0;
			}
		}
		
		inline void set_meridiandata(GNET::Octets &data,const GDB::meridian_data &meridian)
		{
			GNET::GMeridianData tmp;
			tmp.meridian_level = meridian.meridian_level;
			tmp.lifegate_times = meridian.lifegate_times;
			tmp.deathgate_times = meridian.deathgate_times;
			tmp.free_refine_times = meridian.free_refine_times;
			tmp.paid_refine_times = meridian.paid_refine_times;
			tmp.player_login_time = meridian.player_login_time;
			tmp.continu_login_days = meridian.continu_login_days;
			tmp.trigrams_map1 = meridian.trigrams_map[0];
			tmp.trigrams_map2 = meridian.trigrams_map[1];
			tmp.trigrams_map3 = meridian.trigrams_map[2];
			data = Marshal::OctetsStream() << tmp;
		}

		inline void get_meridiandata(const GNET::Octets &data, GDB::meridian_data &meridian)
		{
			if(data.size())
			{
				GNET::GMeridianData tmp;
				try
				{
					Marshal::OctetsStream(data) >> tmp;
					meridian.meridian_level = tmp.meridian_level;
					meridian.lifegate_times = tmp.lifegate_times;
					meridian.deathgate_times = tmp.deathgate_times;
					meridian.free_refine_times = tmp.free_refine_times;
					meridian.paid_refine_times = tmp.paid_refine_times;
					meridian.player_login_time = tmp.player_login_time;
					meridian.continu_login_days = tmp.continu_login_days;
					meridian.trigrams_map[0] = tmp.trigrams_map1;
					meridian.trigrams_map[1] = tmp.trigrams_map2;
					meridian.trigrams_map[2] = tmp.trigrams_map3;
				}catch(Marshal::Exception)
				{
					memset(&meridian,0,sizeof(GDB::meridian_data));
				}
			}
			else
			{
				memset(&meridian,0,sizeof(GDB::meridian_data));
			}
		}
			
		inline void set_kingdata(GNET::Octets& data, int is_king, int king_expire_time)
		{
			if(is_king)
			{
				data = Marshal::OctetsStream() << is_king << king_expire_time;
			}
		}
		inline void get_kingdata(const GNET::Octets& data, int & is_king, int & king_expire_time)
		{
			if(data.size())
			{
				try{
					Marshal::OctetsStream(data) >> is_king >> king_expire_time;
				}catch(Marshal::Exception)
				{
					is_king = king_expire_time = 0;
				}
			}
			else
			{
				is_king = king_expire_time = 0;
			}
		}
		inline void get_extraprop(const GNET::GRoleStatusExtraProp& extraprop, base_info& info, vecdata& data)
		{
				std::map<int, Octets>::const_iterator it;
				for(it = extraprop.data.begin(); it != extraprop.data.end(); ++it)
				{
					try
					{
						switch(it->first)
						{
							case GROLE_STATUS_EXTRAPROP_TOUCH_HALF_TRADE:
								{
									Marshal::OctetsStream(it->second)
									>> info.touch_trade.sn
									>> info.touch_trade.state
									>> info.touch_trade.cost
									>> info.touch_trade.lots
									>> info.touch_trade.itemcount
									>> info.touch_trade.itemtype
									>> info.touch_trade.itemexpire;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_DAILY_SIGN_IN:
								{
									Marshal::OctetsStream(it->second)
									>> info.daily_signin.update_time
									>> info.daily_signin.month_calendar
									>> info.daily_signin.curr_year_data
									>> info.daily_signin.last_year_data
                                    >> info.daily_signin.awarded_times
                                    >> info.daily_signin.late_signin_times
									>> info.daily_signin.reserved;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_GIFTCARD_HALF_REDEEM:
								{
									Octets cardnumber;
									Marshal::OctetsStream(it->second)
									>> info.giftcard_redeem.state
									>> info.giftcard_redeem.type
									>> info.giftcard_redeem.parenttype
									>> cardnumber;
									if(cardnumber.size() != sizeof(info.giftcard_redeem.cardnumber)) throw Marshal::Exception();
									memcpy(info.giftcard_redeem.cardnumber,cardnumber.begin(),cardnumber.size());
								}
								break;
							case GROLE_STATUS_EXTRAPROP_LEADERSHIP:
								{
									Marshal::OctetsStream(it->second) >> info.leadership;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_GENERALCARD_COLLECTION:
								{
									const Octets & collection = it->second;
									get_buf(collection, data.generalcard_collection);
								}
								break;
							case GROLE_STATUS_EXTRAPROP_FATERING:
								{
									const Octets & fatering = it->second;
									get_buf(fatering, data.fatering_data);
								}
								break;
							case GROLE_STATUS_EXTRAPROP_CLOCK_DATA:
								{
									const Octets & clockdata = it->second;
									get_buf(clockdata, data.clock_data);
								}
								break;
							case GROLE_STATUS_EXTRAPROP_RAND_MALL_DATA:
								{
									const Octets & randmalldata = it->second;
									get_buf(randmalldata, data.rand_mall_data);
								}
								break;
							case GROLE_STATUS_EXTRAPROP_WORLD_CONTRIBUTION:
								{
									Marshal::OctetsStream(it->second) >> info.world_contribution.contrib >> info.world_contribution.cost;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_ASTROLABE_EXTERN:
								{
									Marshal::OctetsStream(it->second) >> info.astrolabe_extern.level >> info.astrolabe_extern.exp;
								}
								break;
                            case GROLE_STATUS_EXTRAPROP_SOLO_CHALLENGE_INFO:
                                {
                                    Octets award_info;
                                    Marshal::OctetsStream(it->second)
                                        >> info.solo_challenge_info.max_stage_level
                                        >> info.solo_challenge_info.max_stage_cost_time
                                        >> info.solo_challenge_info.total_score
                                        >> info.solo_challenge_info.total_time
                                        >> info.solo_challenge_info.left_draw_award_times
                                        >> info.solo_challenge_info.playmodes
                                        >> award_info;

                                    if (award_info.size() != sizeof(info.solo_challenge_info.award_info)) throw Marshal::Exception();
                                    memcpy(info.solo_challenge_info.award_info, award_info.begin(), award_info.size());
                                }
                                break;
                            case GROLE_STATUS_EXTRAPROP_MNFACTION_INFO:
                                {
                                    // 跨服情况下以extraprop的数据初始化，本服以userfaction数据初始化
									if(info.mnfaction_info.unifid == 0) 
									{	
										Marshal::OctetsStream(it->second)
                                        	>> info.mnfaction_info.unifid;
									}
                                }
                                break;
                            case GROLE_STATUS_EXTRAPROP_VISA_INFO:
                                {
                                    Marshal::OctetsStream(it->second)
                                        >> info.visa_info.type
                                        >> info.visa_info.stay_timestamp
										>> info.visa_info.cost_item
										>> info.visa_info.cost_item_count;
                                }
                                break;
							case GROLE_STATUS_EXTRAPROP_FIX_POSITION_TRANSMIT_INFO:
								{
									const Octets & fixpositiontransmitdata = it->second;
									get_buf(fixpositiontransmitdata, data.fix_position_transmit_data);
								}
								break;
                            case GROLE_STATUS_EXTRAPROP_CASH_RESURRECT_INFO:
                                {
                                    Marshal::OctetsStream(it->second)
                                        >> info.cash_resurrect_info.times;
                                }
                                break;

                            default:
                                break;
						}
					}
					catch(Marshal::Exception)
					{
						switch(it->first)
						{
							case GROLE_STATUS_EXTRAPROP_TOUCH_HALF_TRADE:
								{
									info.touch_trade.sn = 0;
									info.touch_trade.state = 0;
									info.touch_trade.cost = 0;
									info.touch_trade.lots = 0;
									info.touch_trade.itemcount = 0;
									info.touch_trade.itemtype = 0;
									info.touch_trade.itemexpire = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_DAILY_SIGN_IN:
								{
								 	info.daily_signin.update_time = 0;
									info.daily_signin.month_calendar = 0;
									info.daily_signin.curr_year_data = 0;
									info.daily_signin.last_year_data = 0;
                                    info.daily_signin.awarded_times = 0;
                                    info.daily_signin.late_signin_times = 0;
									info.daily_signin.reserved = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_GIFTCARD_HALF_REDEEM:
								{
									info.giftcard_redeem.state = 0;
									info.giftcard_redeem.type = 0;
									info.giftcard_redeem.parenttype = 0;
									memset(info.giftcard_redeem.cardnumber,0,sizeof(info.giftcard_redeem.cardnumber));
								}
								break;
							case GROLE_STATUS_EXTRAPROP_LEADERSHIP:
								{
									info.leadership = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_GENERALCARD_COLLECTION:
								{
									data.generalcard_collection.data = NULL;
									data.generalcard_collection.size = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_CLOCK_DATA:
								{
									data.clock_data.data = NULL;
									data.clock_data.size = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_RAND_MALL_DATA:
								{
									data.rand_mall_data.data = NULL;
									data.rand_mall_data.size = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_WORLD_CONTRIBUTION:
								{
									info.world_contribution.contrib = 0;
									info.world_contribution.cost = 0;
								}
								break;
							case GROLE_STATUS_EXTRAPROP_ASTROLABE_EXTERN:
								{
									info.astrolabe_extern.level = 0;
									info.astrolabe_extern.exp = 0;
								}
                                break;
                            case GROLE_STATUS_EXTRAPROP_SOLO_CHALLENGE_INFO:
                                {
                                    info.solo_challenge_info.max_stage_level = 0;
                                    info.solo_challenge_info.max_stage_cost_time = 0;
                                    info.solo_challenge_info.total_score = 0;
                                    info.solo_challenge_info.total_time = 0;
                                    info.solo_challenge_info.left_draw_award_times = 0;
                                    info.solo_challenge_info.playmodes = 0;

                                    memset(info.solo_challenge_info.award_info, 0, sizeof(info.solo_challenge_info.award_info));
                                }
                                break;
                            case GROLE_STATUS_EXTRAPROP_MNFACTION_INFO:
                                {
                                    info.mnfaction_info.unifid = 0;
                                }
                                break;
                            case GROLE_STATUS_EXTRAPROP_VISA_INFO:
                                {
                                    info.visa_info.type = 0;
                                    info.visa_info.stay_timestamp = 0;
									info.visa_info.cost_item = 0;
									info.visa_info.cost_item_count = 0;
                                }
                                break;
							case GROLE_STATUS_EXTRAPROP_FIX_POSITION_TRANSMIT_INFO:
								{
									data.fix_position_transmit_data.data = NULL;
									data.fix_position_transmit_data.size = 0;
								}
								break;
                            case GROLE_STATUS_EXTRAPROP_CASH_RESURRECT_INFO:
                                {
                                    info.cash_resurrect_info.times = 0;
                                }
                                break;

                            default:
                                break;
						}

						Log::log(LOG_ERR,"unmarshal extraprop[%d] failed for %d.",it->first, info.id);
					}
				}
		}
		inline void  set_extraprop(GNET::Octets& odata,const base_info& info, const vecdata & data)
		{
			GRoleStatusExtraProp extraprop;

			if(info.touch_trade.state)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_TOUCH_HALF_TRADE]
					= Marshal::OctetsStream()
						<< info.touch_trade.sn
						<< info.touch_trade.state
						<< info.touch_trade.cost
						<< info.touch_trade.lots
						<< info.touch_trade.itemcount
						<< info.touch_trade.itemtype
						<< info.touch_trade.itemexpire;
			}

			extraprop.data[GROLE_STATUS_EXTRAPROP_DAILY_SIGN_IN]
				= Marshal::OctetsStream()
					<< info.daily_signin.update_time
					<< info.daily_signin.month_calendar
					<< info.daily_signin.curr_year_data
					<< info.daily_signin.last_year_data
                    << info.daily_signin.awarded_times
                    << info.daily_signin.late_signin_times
					<< info.daily_signin.reserved;

			if(info.giftcard_redeem.state)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_GIFTCARD_HALF_REDEEM]
					= Marshal::OctetsStream()
						<< info.giftcard_redeem.state
						<< info.giftcard_redeem.type
						<< info.giftcard_redeem.parenttype
						<< Octets(info.giftcard_redeem.cardnumber,sizeof(info.giftcard_redeem.cardnumber));
			}

			extraprop.data[GROLE_STATUS_EXTRAPROP_LEADERSHIP]
				= Marshal::OctetsStream() << info.leadership;
			
			if(data.generalcard_collection.size)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_GENERALCARD_COLLECTION]
					= Octets(data.generalcard_collection.data, data.generalcard_collection.size);
			}

			if(data.fatering_data.size)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_FATERING]
					= Octets(data.fatering_data.data, data.fatering_data.size);
			}
			
			if(data.clock_data.size)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_CLOCK_DATA]
					= Octets(data.clock_data.data, data.clock_data.size);
			}

			if(data.rand_mall_data.size)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_RAND_MALL_DATA]
					= Octets(data.rand_mall_data.data, data.rand_mall_data.size);
			}

			extraprop.data[GROLE_STATUS_EXTRAPROP_WORLD_CONTRIBUTION]
				= Marshal::OctetsStream() << info.world_contribution.contrib << info.world_contribution.cost;

			if(info.astrolabe_extern.level || info.astrolabe_extern.exp)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_ASTROLABE_EXTERN]
					= Marshal::OctetsStream() << info.astrolabe_extern.level << info.astrolabe_extern.exp;
			}

            if (info.solo_challenge_info.max_stage_level != 0)
            {
                extraprop.data[GROLE_STATUS_EXTRAPROP_SOLO_CHALLENGE_INFO]
                    = (Marshal::OctetsStream()
                        << info.solo_challenge_info.max_stage_level
                        << info.solo_challenge_info.max_stage_cost_time
                        << info.solo_challenge_info.total_score
                        << info.solo_challenge_info.total_time
                        << info.solo_challenge_info.left_draw_award_times
                        << info.solo_challenge_info.playmodes
                        << Octets(info.solo_challenge_info.award_info, sizeof(info.solo_challenge_info.award_info)));
            }

            if (info.mnfaction_info.unifid != 0)
            {
                extraprop.data[GROLE_STATUS_EXTRAPROP_MNFACTION_INFO]
                    = (Marshal::OctetsStream()
                        << info.mnfaction_info.unifid);
            }

            if (info.visa_info.stay_timestamp != 0)
            {
                extraprop.data[GROLE_STATUS_EXTRAPROP_VISA_INFO]
                    = (Marshal::OctetsStream()
                        << info.visa_info.type
                        << info.visa_info.stay_timestamp
						<< info.visa_info.cost_item
						<< info.visa_info.cost_item_count);
            }

			if(data.fix_position_transmit_data.size)
			{
				extraprop.data[GROLE_STATUS_EXTRAPROP_FIX_POSITION_TRANSMIT_INFO] = Octets(data.fix_position_transmit_data.data, data.fix_position_transmit_data.size);
			}

            if (info.cash_resurrect_info.times >= 0)
            {
                extraprop.data[GROLE_STATUS_EXTRAPROP_CASH_RESURRECT_INFO]
                    = (Marshal::OctetsStream()
                        << info.cash_resurrect_info.times);
            }

			odata = Marshal::OctetsStream() << extraprop;
		}

		inline void set_reincarnationdata(GNET::Octets & data, const GDB::reincarnation_data & rdata)
		{
			GNET::GReincarnationData tmpdata;
			tmpdata.tome_exp 	= rdata.tome_exp;
			tmpdata.tome_active = rdata.tome_active;
			for(size_t i=0; i<rdata.count; i++)
			{
				GNET::GReincarnationRecord tmprecord;
				tmprecord.level 	= rdata.records[i].level;
				tmprecord.timestamp	= rdata.records[i].timestamp;
				tmprecord.exp		= rdata.records[i].exp;
				tmpdata.records.push_back(tmprecord);
			}
			data = Marshal::OctetsStream() << tmpdata;	
		}
		inline void get_reincarnationdata(const GNET::Octets & odata, GDB::reincarnation_data & rdata)	
		{
			GReincarnationData data;
			try{
				if(odata.size())
				{
					Marshal::OctetsStream(odata) >> data;
				}
			}
			catch(Marshal::Exception)
			{
				data.tome_exp = 0;
				data.tome_active = 0;
				data.records.clear();
			}
			
			rdata.tome_exp 		= data.tome_exp;
			rdata.tome_active 	= data.tome_active;
			rdata.count 		= data.records.size();
			rdata.records 		= NULL;
			if(rdata.count == 0) return;
			rdata.records 		= (GDB::reincarnation_record *)malloc(sizeof(GDB::reincarnation_record) * rdata.count);
			for(size_t i=0; i<rdata.count; i++)
			{
				const GNET::GReincarnationRecord & record = data.records[i];
				rdata.records[i].level 		= record.level;
				rdata.records[i].timestamp 	= record.timestamp;
				rdata.records[i].exp 		= record.exp;
			}
		}
		inline void release_reincarnationdata(GDB::reincarnation_data & rdata)
		{
			if(rdata.count)
			{
				free(rdata.records);
				rdata.count 	= 0;
				rdata.records 	= NULL;
			}
		}

		inline void set_realmdata(GNET::Octets &data,const GDB::realm_data &realm)
		{
			data = Marshal::OctetsStream() << realm.level << realm.exp << realm.reserved1 << realm.reserved2;
		}

		inline void get_realmdata(const GNET::Octets &data, GDB::realm_data &realm)
		{
			if(data.size())
			{
				try
				{
					Marshal::OctetsStream(data) >> realm.level >> realm.exp >> realm.reserved1 >> realm.reserved2;
				}catch(Marshal::Exception)
				{
					memset(&realm,0,sizeof(GDB::realm_data));
				}
			}
			else
			{
				memset(&realm,0,sizeof(GDB::realm_data));
			}
		}

		inline void set_rankdata(GNET::Octets &data,const GDB::rank_data &rank)
		{
			data = Marshal::OctetsStream() << rank.point << rank.kill << rank.dead;
		}

		inline void get_rankdata(const GNET::Octets &data, GDB::rank_data &rank)
		{
			if(data.size())
			{
				try
				{
					Marshal::OctetsStream(data) >> rank.point >> rank.kill >> rank.dead;
				}catch(Marshal::Exception)
				{
					memset(&rank,0,sizeof(GDB::rank_data));
				}
			}
			else
			{
				memset(&rank,0,sizeof(GDB::rank_data));
			}
		}
};

	bool Role2Info(GNET::GRoleDetail * pRole,base_info& info,vecdata& data,int data_mask,
			const GPetCorral& corral, const GRoleStatusExtraProp & extraprop)
	{
		if(data_mask!=GET_ALL)
			return false;
		memset(&info,0,sizeof(info));
		memset(&data,0,sizeof(data));
		info.id = pRole->id;
		info.userid = pRole->userid;
		info.race = pRole->race;
		info.cls = pRole->cls;
		info.gender = pRole->gender;
		info.trashbox_active = ((data_mask&0x00000001)!=0);
		info.level = pRole->status.level;
		info.sec_level = pRole->status.level2;
		info.exp = pRole->status.exp;
		info.sp = pRole->status.sp;
		info.pp = pRole->status.pp;
		info.hp = pRole->status.hp;
		info.mp = pRole->status.mp;
		info.posx = pRole->status.posx;
		info.posy = pRole->status.posy;
		info.posz = pRole->status.posz;
		info.worldtag = pRole->status.worldtag;
		info.money = pRole->inventory.money;
		info.custom_crc = pRole->custom_stamp;
		info.invader_state = pRole->status.invader_state ;
		info.invader_time = pRole->status.invader_time ;
		info.pariah_time = pRole->status.pariah_time;
		info.factionid = pRole->factionid;
		info.factionrole = pRole->factionrole;
		info.reputation = pRole->status.reputation;
		info.dbltime_expire = pRole->status.dbltime_expire;
		info.dbltime_mode = pRole->status.dbltime_mode;
		info.dbltime_begin = pRole->status.dbltime_begin;
		info.dbltime_used = pRole->status.dbltime_used;
		info.dbltime_max = pRole->status.dbltime_max;
		info.time_used = pRole->status.time_used;
		info.create_time = pRole->create_time;
		info.lastlogin_time = pRole->lastlogin_time;
		info.storesize = pRole->storehouse.capacity;
		info.storesize1 = pRole->storehouse.size1;
		info.storesize2 = pRole->storehouse.size2;
		info.storesize3 = pRole->storehouse.size3;
		info.bagsize = pRole->inventory.capacity;
		info.timestamp = pRole->inventory.timestamp;
		info.cash_add = pRole->cash_add;
		info.cash_total = pRole->cash_total;
		info.cash_delta = 0;
		info.cash_used = pRole->cash_used;
		info.cash_serial = pRole->cash_serial;
		info.cash = info.cash_total - info.cash_used;
		info.spouse = pRole->spouse;
		info.bonus_add = pRole->bonus_add;
		info.bonus_reward = pRole->bonus_reward;
		info.bonus_used = pRole->bonus_used;
		info.referrer = pRole->referrer;
		info.mall_consumption = pRole->mall_consumption;
		info.src_zoneid = pRole->src_zoneid;
		info.mnfaction_info.unifid = pRole->unifid;
		info.vip_level = pRole->vip_level;
		info.score_add = pRole->score_add;
		info.score_cost = pRole->score_cost;
		info.score_consume = pRole->score_consume;
		info.next_day_item_clear_timestamp    = pRole->day_clear_stamp;
		info.next_week_item_clear_timestamp   = pRole->week_clear_stamp;
		info.next_month_item_clear_timestamp  = pRole->month_clear_stamp;
		info.next_year_item_clear_timestamp   = pRole->year_clear_stamp;

		get_buf(pRole->name,data.user_name);
		get_buf(pRole->status.custom_status,data.custom_status);
		get_buf(pRole->status.filter_data,data.filter_data);
		get_buf(pRole->status.charactermode,data.charactermode);
		get_buf(pRole->status.instancekeylist,data.instancekeylist);
		get_buf(pRole->status.property,data.property);
		get_buf(pRole->status.var_data,data.var_data);
		get_buf(pRole->status.skills,data.skill_data);
		get_buf(pRole->task.task_data,data.task_data);
		get_buf(pRole->task.task_complete,data.finished_task_data);
		get_buf(pRole->task.task_finishtime,data.finished_time_task_data);
		get_buf(pRole->status.storehousepasswd,data.trashbox_passwd);
		get_buf(pRole->status.waypointlist,data.waypoint_list);
		get_buf(pRole->status.coolingtime,data.coolingtime);
		get_buf(pRole->status.dbltime_data,data.dbltime_data);
		get_buf(pRole->status.npc_relation,data.npc_relation);
		get_buf(pRole->addiction,data.addiction_data);
		get_buf(pRole->taskcounter,data.task_counter);
		get_buf(pRole->status.multi_exp_ctrl,data.multi_exp_ctrl);
		get_buf(pRole->status.storage_task,data.storage_task);
		get_buf(pRole->status.faction_contrib,data.faction_contrib);
		get_buf(pRole->status.online_award,data.online_award);
		get_buf(pRole->status.profit_time_data, data.profit_time_data);
		get_buf(pRole->status.title_data, data.title_data);

		get_itemlist(pRole->inventory.items,data.inventory);
		get_itemlist(pRole->equipment,data.equipment);
		get_itemlist(pRole->task.task_inventory,data.task_inventory);
		get_petcorral(corral,data.pets);
		if(data_mask & GET_STOREHOUSE)
		{
			get_itemlist(pRole->storehouse.items,data.trash_box);
			get_itemlist(pRole->storehouse.dress,data.trash_box1);
			get_itemlist(pRole->storehouse.material,data.trash_box2);
			get_itemlist(pRole->storehouse.generalcard,data.trash_box3);
			info.trash_money = pRole->storehouse.money;
		}
		info.userstore_active = true;//默认情况一定取账号仓库数据
		info.userstoresize = pRole->userstorehouse.capacity;
		info.userstoremoney = pRole->userstorehouse.money;
		get_itemlist(pRole->userstorehouse.items,data.user_store);
		get_factionrelation(pRole->factionalliance, pRole->factionhostile, data.faction_relation);
		get_forcedata(pRole->status.force_data,data.force_data);
		get_countrydata(pRole->status.country_data, info.country_id, info.country_expire_time);
		get_meridiandata(pRole->status.meridian_data,data.meridian);
		get_kingdata(pRole->status.king_data, info.is_king, info.king_expire_time);
		get_extraprop(extraprop, info, data);
		get_reincarnationdata(pRole->status.reincarnation_data, data.reincarnation);
		get_realmdata(pRole->status.realm_data, data.realm);
		get_rankdata(pRole->status.rank, data.rank);
	
		get_buf(pRole->purchase_limit_data, data.purchase_limit_data);
		return true;
	}

	void InventoryToItemList(const GNET::GRoleInventoryVector & backpack, itemlist & list)
	{
		return get_itemlist(backpack,list);
	}

	void ReleaseItemList( itemlist & list)
	{
		return release_itemlist(list);
	}
	

	void ReleaseAllInventory(vecdata & data)
	{
		release_itemlist(data.inventory);
		release_itemlist(data.equipment);
		release_itemlist(data.task_inventory);
		release_itemlist(data.trash_box);
		release_itemlist(data.trash_box1);
		release_itemlist(data.trash_box2);
		release_itemlist(data.trash_box3);
		release_itemlist(data.user_store);
		release_petcorral(data.pets);
		release_factionrelation(data.faction_relation);
		release_forcedata(data.force_data);
		release_reincarnationdata(data.reincarnation);
	}

	void Info2Role(GNET::GRoleDetail * pRole,const base_info & info,const vecdata & data)
	{
		pRole->id = info.id;
		pRole->userid = info.userid;
		pRole->race = info.race;
		pRole->cls = info.cls;
		pRole->gender = info.gender;
		pRole->status.level = info.level;
		pRole->status.level2 = info.sec_level;
		pRole->status.exp = info.exp;
		pRole->status.sp = info.sp;
		pRole->status.pp = info.pp;
		pRole->status.hp = info.hp;
		pRole->status.mp = info.mp;
		pRole->status.posx = info.posx;
		pRole->status.posy = info.posy;
		pRole->status.posz = info.posz;
		pRole->status.worldtag = info.worldtag;
		pRole->inventory.money = info.money;
		pRole->status.invader_state = info.invader_state; 
		pRole->status.invader_time  = info.invader_time;
		pRole->status.pariah_time   = info.pariah_time;
		pRole->factionid     = info.factionid;
		pRole->factionrole   = info.factionrole;
		pRole->status.reputation    = info.reputation;
		pRole->status.dbltime_expire = info.dbltime_expire;
		pRole->status.dbltime_mode = info.dbltime_mode;
		pRole->status.dbltime_begin = info.dbltime_begin;
		pRole->status.dbltime_used = info.dbltime_used;
		pRole->status.dbltime_max = info.dbltime_max;
		pRole->status.time_used = info.time_used;
		pRole->storehouse.capacity = info.storesize;
		pRole->storehouse.size1 = info.storesize1;
		pRole->storehouse.size2 = info.storesize2;
		pRole->storehouse.size3 = info.storesize3;
		pRole->inventory.capacity = info.bagsize;
		pRole->inventory.timestamp = info.timestamp;
		pRole->cash_used = info.cash_used - info.cash_delta;
		pRole->cash_serial = info.cash_serial;
		pRole->bonus_used = info.bonus_used;
		pRole->mall_consumption = info.mall_consumption;
		pRole->src_zoneid = info.src_zoneid;
		pRole->vip_level         = info.vip_level;
		pRole->score_add         = info.score_add;
		pRole->score_cost        = info.score_cost;
		pRole->score_consume	 = info.score_consume;
		pRole->day_clear_stamp   = info.next_day_item_clear_timestamp;
		pRole->week_clear_stamp  = info.next_week_item_clear_timestamp;
		pRole->month_clear_stamp = info.next_month_item_clear_timestamp;
		pRole->year_clear_stamp  = info.next_year_item_clear_timestamp;
		
		push_back(pRole->status.custom_status,data.custom_status);
		push_back(pRole->status.filter_data,data.filter_data);
		push_back(pRole->status.charactermode,data.charactermode);
		push_back(pRole->status.instancekeylist,data.instancekeylist);
		push_back(pRole->status.property,data.property);
		push_back(pRole->task.task_data,data.task_data);
		push_back(pRole->task.task_complete,data.finished_task_data);
		push_back(pRole->task.task_finishtime,data.finished_time_task_data);
		push_back(pRole->status.var_data,data.var_data);
		push_back(pRole->status.skills,data.skill_data);
		push_back(pRole->status.storehousepasswd,data.trashbox_passwd);
		push_back(pRole->status.waypointlist,data.waypoint_list);
		push_back(pRole->status.coolingtime,data.coolingtime);
		push_back(pRole->status.dbltime_data,data.dbltime_data);
		push_back(pRole->status.npc_relation,data.npc_relation);
		push_back(pRole->addiction,data.addiction_data);
		push_back(pRole->taskcounter,data.task_counter);
		push_back(pRole->status.multi_exp_ctrl,data.multi_exp_ctrl);
		push_back(pRole->status.storage_task,data.storage_task);
		push_back(pRole->status.faction_contrib,data.faction_contrib);
		push_back(pRole->status.online_award,data.online_award);
		push_back(pRole->status.profit_time_data, data.profit_time_data);
		push_back(pRole->status.title_data,data.title_data);

		set_itemlist(pRole->inventory.items,data.inventory);
		set_itemlist(pRole->equipment,data.equipment);
		set_itemlist(pRole->task.task_inventory,data.task_inventory);
		set_petcorral(pRole->status.petcorral,data.pets);
		set_loglist(info.id,pRole->logs,data.logs);
		if(info.trashbox_active)
		{
			set_itemlist(pRole->storehouse.items,data.trash_box);
			set_itemlist(pRole->storehouse.dress,data.trash_box1);
			set_itemlist(pRole->storehouse.material,data.trash_box2);
			set_itemlist(pRole->storehouse.generalcard,data.trash_box3);
			pRole->storehouse.money = info.trash_money;
		}
		if (info.userstore_active)
		{
			pRole->userstorehouse.capacity = info.userstoresize;
			pRole->userstorehouse.money = info.userstoremoney;
			set_itemlist(pRole->userstorehouse.items,data.user_store);
		}
		set_forcedata(pRole->status.force_data, data.force_data);
		set_countrydata(pRole->status.country_data, info.country_id, info.country_expire_time);
		set_meridiandata(pRole->status.meridian_data,data.meridian);
		set_kingdata(pRole->status.king_data, info.is_king, info.king_expire_time);
		set_extraprop(pRole->status.extraprop, info, data);
		set_reincarnationdata(pRole->status.reincarnation_data, data.reincarnation);
		set_realmdata(pRole->status.realm_data, data.realm);
		set_rankdata(pRole->status.rank, data.rank);

		push_back(pRole->purchase_limit_data, data.purchase_limit_data);
	}

	size_t
	convert_item(const GRoleInventoryVector & ivec, itemdata * list , size_t size)
	{
		if(ivec.size() == 0) return 0;
		size_t count = ivec.size();
		if(count > size) count = size;
		for(size_t i = 0; i < count; i ++)
		{
			itemdata * pItem = list + i;
			const GRoleInventory &obj = ivec[i]; 
			pItem->id 	= obj.id;
			pItem->index 	= obj.pos;
			pItem->count 	= obj.count;
			pItem->max_count= obj.max_count;
			pItem->guid1 	= obj.guid1;
			pItem->guid2 	= obj.guid2;
			pItem->mask 	= obj.mask;
			pItem->proctype = obj.proctype;
			pItem->expire_date = obj.expire_date;
			pItem->data	= obj.data.begin();
			pItem->size	= obj.data.size();
		}
		return count;
	}
	
	void 
	itemlist_to_inventory(GNET::RpcDataVector<GNET::GRoleInventory> & ivec, const itemlist & list)
	{
		return set_itemlist(ivec, list);
	}

	bool put_role(int id,const base_info* pInfo,const vecdata* pData,Result* callback,int priority,int mask)
	{
		RolePair arg;
		arg.key.id = id;
		arg.priority = priority;
		arg.data_mask = mask;
		if(!pInfo->trashbox_active)
			arg.data_mask ^= DBMASK_PUT_STOREHOUSE;
		if(!pInfo->userstore_active)
			arg.data_mask ^= DBMASK_PUT_USERSTORE;

		Info2Role(&arg.value,*pInfo,*pData);
		Rpc *rpc = Rpc::Call(RPC_PUTROLE, &arg);
		((PutRole *)rpc)->_callback = callback;
		return GamedbClient::GetInstance()->SendProtocol(*rpc);
	}

	bool get_role(int id, Result * callback)
	{
		RoleArg arg( RoleId((unsigned int )id) , 0x0000003F);
		Rpc *rpc = Rpc::Call(RPC_GETROLE, &arg);
		((GetRole *)rpc)->_callback = callback;
		bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
		if(success) PollIO::WakeUp();
		return success;
	}

	bool put_money_inventory(int id, size_t money, itemlist & list,Result * callback)
	{
		PutMoneyInventoryArg arg;
		arg.roleid = id;
		arg.money = money;
		set_itemlist(arg.goods,list);

		Rpc *rpc = Rpc::Call(RPC_PUTMONEYINVENTORY, &arg);
		((PutMoneyInventory*)rpc)->_callback = callback;
		return GamedbClient::GetInstance()->SendProtocol(*rpc);
	}

	bool get_money_inventory(int id, Result * callback, int mask)
	{
		Rpc *rpc = Rpc::Call(RPC_GETMONEYINVENTORY, GetMoneyInventoryArg(id, mask));
		((GetMoneyInventory*)rpc)->_callback = callback;
		bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
		if(success) PollIO::WakeUp();
		return success;

	}

	bool get_cash_total(int userid, Result * callback)
	{
		Rpc *rpc = Rpc::Call(RPC_GETCASHTOTAL, GetCashTotalArg(userid));
		((GetCashTotal*)rpc)->_callback = callback;
		bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
		if(success) PollIO::WakeUp();
		return success;
	}
	
	bool init_gamedb()
	{
		Conf *conf = Conf::GetInstance();
		GamedbClient *manager = GamedbClient::GetInstance();
		manager->SetAccumulate(atoi(conf->find(manager->Identification(), "accumulate").c_str()));
		Protocol::Client(manager);
		return true;
	}

	bool set_couple(int id1, int id2, int op)
	{
		Rpc *rpc = Rpc::Call(RPC_PUTSPOUSE, PutSpouseArg(op,id1,id2));
		bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
		if(success) PollIO::WakeUp();
		return success;
	}
	
	void ServerData2DB(GNET::GServerData * pData, const serverdata & data)
	{
		pData->world_tag = data.world_tag;
		push_back(pData->wedding_data, data.wedding_data);
		push_back(pData->dpsrank_data, data.dpsrank_data);
	}
	
	void DB2ServerData(const GNET::GServerData * pData, serverdata & data)
	{
		data.world_tag = pData->world_tag;
		get_buf(pData->wedding_data, data.wedding_data);
		get_buf(pData->dpsrank_data, data.dpsrank_data);
	}
	
	bool put_serverdata(int world_tag, const serverdata* data, ServerDataResult * callback, int priority, int mask)
	{
		PutServerDataArg arg;
		arg.world_tag = world_tag;
		arg.data_mask = mask;
		arg.priority = priority;
		ServerData2DB(&arg.data, *data);
		Rpc * rpc = Rpc::Call(RPC_PUTSERVERDATA,arg);
		((PutServerData*)rpc)->_callback = callback;
		return GamedbClient::GetInstance()->SendProtocol(*rpc);
	}
	
	bool get_serverdata(int world_tag, ServerDataResult * callback, int mask)
	{
		GetServerDataArg arg(world_tag,mask);
		Rpc * rpc = Rpc::Call(RPC_GETSERVERDATA, arg);
		((GetServerData*)rpc)->_callback = callback;
		bool success = GamedbClient::GetInstance()->SendProtocol(*rpc);
		if(success) PollIO::WakeUp();
		return success;
	}

    bool copy_role(int src_roleid, int dst_roleid, CopyRoleResult* callback)
    {
        DBCopyRole* rpc = (DBCopyRole*)Rpc::Call(RPC_DBCOPYROLE, DBCopyRoleArg(src_roleid, dst_roleid));
        rpc->_callback = callback;
        return GamedbClient::GetInstance()->SendProtocol(rpc);
    }
};

