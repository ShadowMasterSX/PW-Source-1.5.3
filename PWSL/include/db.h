#ifndef __GNET_DB_H
#define __GNET_DB_H

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <functional>
#include <algorithm>

#include "gnetalloc.h"

namespace GNET
{

namespace __db_helper
{

typedef unsigned int   page_index_t;
typedef unsigned short page_pos_t;
typedef unsigned short word_t;
typedef unsigned int   dword_t;
typedef unsigned char  byte_t;
typedef unsigned int   lru_t;
typedef unsigned int   counter_t;
typedef unsigned int   size32_t;

class Performance;
class Page;
class PagePool;
class PageFile;
class PageMemory;
class Logger;
class NullLogger;
class PageLogger;
class GlobalLogger;
class PageHash;
class PageCache;
class PageBrowser;
class PageRebuild;
class PerformanceMonitor;
class PageMonitor;
class LoggerMonitor;
class IQueryKey;
class IQueryData;

#ifndef PAGESIZE
#define PAGESIZE	4096
#endif

#ifndef MEMSLOTSIZE
#define MEMSLOTSIZE	1024
#endif

#ifndef PAGEPOOLSIZE
#define PAGEPOOLSIZE	4096
#endif

#define STACKBUFFER	65536

#define PAGEUSED	(PAGESIZE - 8)
#define PAGEMASK	(~((ptrdiff_t)PAGESIZE-1))

#pragma pack(1)

class Performance
{
	counter_t _page_alloc;
	counter_t _page_free;
	counter_t _page_load_hit;
	counter_t _page_load_miss;
	counter_t _find_key;
	counter_t _insert;
	counter_t _insert_replace;
	counter_t _insert_reject;
	counter_t _split;
	counter_t _insert_leaf_adjust_left;
	counter_t _insert_leaf_adjust_right;
	counter_t _insert_internal_adjust_left;
	counter_t _insert_internal_adjust_right;
	counter_t _remove_found;
	counter_t _remove_not_found;
	counter_t _remove_leaf;
	counter_t _remove_internal;
	counter_t _l_shrink_leaf;
	counter_t _l_shrink_internal;
	counter_t _r_shrink_leaf;
	counter_t _r_shrink_internal;
	counter_t _l_shrink_leaf_adjust;
	counter_t _l_shrink_internal_adjust;
	counter_t _r_shrink_leaf_adjust;
	counter_t _r_shrink_internal_adjust;
	counter_t _merge_sibling;
	counter_t _merge_sibling_adjust;
	counter_t _merge_sibling_nest;
	counter_t _page_read;
	counter_t _page_write;
	counter_t _page_sync;
	size32_t  _cache_high;
	size32_t  _cache_low;
	size32_t  _cache_peak;
	size32_t  _dirty_peak;
	size32_t  _snapshot_peak;
public:
	void page_alloc()                   { _page_alloc++;                   }
	void page_free()                    { _page_free++;                    }
	void page_load_hit()                { _page_load_hit++;                }
	void page_load_miss()               { _page_load_miss++;               }
	void find_key()                     { _find_key++;                     }
	void insert()                       { _insert++;                       }
	void insert_replace()               { _insert_replace++;               }
	void insert_reject()                { _insert_reject++;                }
	void remove_found()                 { _remove_found++;                 }
	void remove_not_found()             { _remove_not_found++;             }
	void split()                        { _split++;                        }
	void insert_leaf_adjust_left()      { _insert_leaf_adjust_left++;      }
	void insert_leaf_adjust_right()     { _insert_leaf_adjust_right++;     }
	void insert_internal_adjust_left()  { _insert_internal_adjust_left++;  }
	void insert_internal_adjust_right() { _insert_internal_adjust_right++; }
	void remove_leaf()                  { _remove_leaf++;                  }
	void remove_internal()              { _remove_internal++;              }
	void l_shrink_leaf()                { _l_shrink_leaf++;                }
	void r_shrink_leaf()                { _r_shrink_leaf++;                }
	void l_shrink_internal()            { _l_shrink_internal++;            }
	void r_shrink_internal()            { _r_shrink_internal++;            }
	void l_shrink_leaf_adjust()         { _l_shrink_leaf_adjust++;         }
	void r_shrink_leaf_adjust()         { _r_shrink_leaf_adjust++;         }
	void l_shrink_internal_adjust()     { _l_shrink_internal_adjust++;     }
	void r_shrink_internal_adjust()     { _r_shrink_internal_adjust++;     }
	void merge_sibling()                { _merge_sibling++;                }
	void merge_sibling_adjust()         { _merge_sibling_adjust++;         }
	void merge_sibling_nest()           { _merge_sibling_nest++;           }
	void set_page_read ( counter_t c )  { _page_read  = c;                 }
	void set_page_write( counter_t c )  { _page_write = c;                 }
	void set_page_sync ( counter_t c )  { _page_sync  = c;                 }
	void set_cache_peak( size_t peak )  { if ( peak > _cache_peak ) _cache_peak = peak; }
	void reset_peak()           	    { _cache_peak = 0, _dirty_peak = 0, _snapshot_peak = 0; }
	void set_cache_high ( size_t cache_high ) { _cache_high = cache_high; }
	void set_cache_low  ( size_t cache_low  ) { _cache_low  = cache_low ; }
	void set_dirty_peak ( size_t peak ) { if ( peak > _dirty_peak ) _dirty_peak = peak; }
	void set_snapshot_peak ( size_t peak ) { if ( peak > _snapshot_peak ) _snapshot_peak = peak; }
	size_t get_cache_high() const { return _cache_high; } 
	size_t get_cache_low () const { return _cache_low;  } 
	size_t record_count() const { return _insert - _remove_found; }

	Performance& operator -= (const Performance &rhs)
	{
		_page_alloc                   -= rhs._page_alloc;
		_page_free                    -= rhs._page_free;
		_page_load_hit                -= rhs._page_load_hit;
		_page_load_miss               -= rhs._page_load_miss;
		_find_key                     -= rhs._find_key;
		_insert                       -= rhs._insert;
		_insert_replace               -= rhs._insert_replace;
		_insert_reject                -= rhs._insert_reject;
		_remove_found                 -= rhs._remove_found;
		_remove_not_found             -= rhs._remove_not_found;
		_split                        -= rhs._split;
		_insert_leaf_adjust_left      -= rhs._insert_leaf_adjust_left;
		_insert_leaf_adjust_right     -= rhs._insert_leaf_adjust_right;
		_insert_internal_adjust_left  -= rhs._insert_internal_adjust_left;
		_insert_internal_adjust_right -= rhs._insert_internal_adjust_right;
		_remove_leaf                  -= rhs._remove_leaf;
		_remove_internal              -= rhs._remove_internal;
		_l_shrink_leaf                -= rhs._l_shrink_leaf;
		_r_shrink_leaf                -= rhs._r_shrink_leaf;
		_l_shrink_internal            -= rhs._l_shrink_internal;
		_r_shrink_internal            -= rhs._r_shrink_internal;
		_l_shrink_leaf_adjust         -= rhs._l_shrink_leaf_adjust;
		_r_shrink_leaf_adjust         -= rhs._r_shrink_leaf_adjust;
		_l_shrink_internal_adjust     -= rhs._l_shrink_internal_adjust;
		_r_shrink_internal_adjust     -= rhs._r_shrink_internal_adjust;
		_merge_sibling                -= rhs._merge_sibling;
		_merge_sibling_adjust         -= rhs._merge_sibling_adjust;
		_merge_sibling_nest           -= rhs._merge_sibling_nest;
		_page_read                    -= rhs._page_read;
		_page_write                   -= rhs._page_write;
		_page_sync                    -= rhs._page_sync;
		return *this;
	}

	void dump( FILE *fp = stdout, double e = 1.0 ) const
	{
		fprintf( fp, "page_alloc                   : %lf\n", _page_alloc / e                  );
		fprintf( fp, "page_free                    : %lf\n", _page_free / e                   );
		fprintf( fp, "page_load_hit                : %lf\n", _page_load_hit / e               );
		fprintf( fp, "page_load_miss               : %lf\n", _page_load_miss / e              );
		fprintf( fp, "find_key                     : %lf\n", _find_key / e                    );

		fprintf( fp, "insert                       : %lf\n", _insert / e                      );
		fprintf( fp, "insert_replace               : %lf\n", _insert_replace / e              );
		fprintf( fp, "insert_reject                : %lf\n", _insert_reject / e               );
		fprintf( fp, "insert_leaf_adjust_left      : %lf\n", _insert_leaf_adjust_left / e     );
		fprintf( fp, "insert_leaf_adjust_right     : %lf\n", _insert_leaf_adjust_right / e    );
		fprintf( fp, "insert_internal_adjust_left  : %lf\n", _insert_internal_adjust_left / e );
		fprintf( fp, "insert_internal_adjust_right : %lf\n", _insert_internal_adjust_right / e);
		fprintf( fp, "split                        : %lf\n", _split / e                       );

		fprintf( fp, "remove_not_found             : %lf\n", _remove_not_found / e            );
		fprintf( fp, "remove_found                 : %lf\n", _remove_found / e                );
		fprintf( fp, "remove_leaf                  : %lf\n", _remove_leaf / e                 );
		fprintf( fp, "remove_internal              : %lf\n", _remove_internal / e             );
		fprintf( fp, "l_shrink_leaf                : %lf\n", _l_shrink_leaf / e               );
		fprintf( fp, "r_shrink_leaf                : %lf\n", _r_shrink_leaf / e               );
		fprintf( fp, "l_shrink_internal            : %lf\n", _l_shrink_internal / e           );
		fprintf( fp, "r_shrink_internal            : %lf\n", _r_shrink_internal / e           );
		fprintf( fp, "l_shrink_leaf_adjust         : %lf\n", _l_shrink_leaf_adjust / e        );
		fprintf( fp, "r_shrink_leaf_adjust         : %lf\n", _r_shrink_leaf_adjust / e        );
		fprintf( fp, "l_shrink_internal_adjust     : %lf\n", _l_shrink_internal_adjust / e    );
		fprintf( fp, "r_shrink_internal_adjust     : %lf\n", _r_shrink_internal_adjust / e    );
		fprintf( fp, "merge_sibling                : %lf\n", _merge_sibling / e               );
		fprintf( fp, "merge_sibling_adjust         : %lf\n", _merge_sibling_adjust / e        );
		fprintf( fp, "merge_sibling_nest           : %lf\n", _merge_sibling_nest / e          );
		fprintf( fp, "page_read                    : %lf\n", _page_read / e                   );
		fprintf( fp, "page_write                   : %lf\n", _page_write / e                  );
		fprintf( fp, "page_sync                    : %lf\n", _page_sync / e                   );
		fprintf( fp, "cache_peak                   : %u\n", _cache_peak                      );
		fprintf( fp, "cache_high                   : %u\n", _cache_high                      );
		fprintf( fp, "cache_low                    : %u\n", _cache_low                       );
		fprintf( fp, "dirty_peak                   : %u\n", _dirty_peak                      );
		fprintf( fp, "snapshot_peak                : %u\n", _snapshot_peak                   );
	}
};

struct data_hdr
{
	page_index_t	next_page_index;
	page_pos_t	next_page_pos;
	page_pos_t	first_slice;
	page_pos_t	next;
	page_pos_t	size;
	data_hdr*	Rnext_page_head();
	data_hdr*	Wnext_page_head();
	data_hdr*	next_head();
	size_t		capacity();
	void		set_next( page_index_t idx, page_pos_t pos ) { next_page_index = idx; next_page_pos = pos; }
};

#define FRAGMIN		8
#define PAGEMAXSPARE	( PAGEUSED - sizeof( data_hdr ) )
#define PAGEMINSPARE	( FRAGMIN  + sizeof( data_hdr ) )
struct frag_hdr
{
	page_index_t	next_page_index;
	page_index_t	prev_page_index;
	page_pos_t	next;			// according to data_hdr::next
	page_pos_t	size;			// according to data_hdr::size must be 0
	page_pos_t	next_page_pos;
	page_pos_t	prev_page_pos;
	frag_hdr*	Wnext_page_head();
	frag_hdr*	Wprev_page_head();
	void		set_next( page_index_t idx, page_pos_t pos ) { next_page_index = idx; next_page_pos = pos; }
	void		set_prev( page_index_t idx, page_pos_t pos ) { prev_page_index = idx; prev_page_pos = pos; }
};

struct index_hdr
{
	page_index_t	child_index;
	union
	{
		struct
		{
			page_index_t	page_index;
			byte_t		key[5];
			byte_t		key_len;
			page_pos_t	page_pos;
		};

		struct
		{
			page_index_t	parent_index;
			page_pos_t	parent_pos;
			page_pos_t	l_pos;
			page_pos_t	r_pos;
		};
	};
	data_hdr*	Rindex_hdr2data_hdr();
	data_hdr*	Windex_hdr2data_hdr();
	void		set_data_position( page_index_t idx, page_pos_t pos ) { page_index = idx; page_pos = pos; }
};

struct logger_hdr
{
	page_index_t	logger_first_idx;
	page_index_t	logger_last_idx;
	time_t		logger_check_timestamp;
#define LCR_NULL	0
#define LCR_PREPARED	1
#define LCR_COMMIT	2
#define LCR_ABORT	3
	page_index_t	check_result;			
};

union PageLayout
{
	char dummy_fill[PAGEUSED];
	union
	{
		struct
		{
			page_index_t	free_page_list;
			page_index_t	max_page_idx;
			page_index_t	root_index_idx;
			time_t		logger_last_check;
			page_index_t	logger_first_idx;
			page_index_t	logger_magic_idx;
			time_t		logger_id;
			page_index_t	logger_serial;
			Performance	performance;
			page_index_t	frag_page_index[ PAGEUSED / FRAGMIN ];
			page_pos_t	frag_page_pos  [ PAGEUSED / FRAGMIN ];
		};

#define INDEXCOUNT	((PAGEUSED / sizeof(index_hdr)) - 1)
#define INDEXMINL	( INDEXCOUNT / 2 )
#define INDEXMINR	( INDEXCOUNT - INDEXMINL - 1 )
#define INDEXSLIPR	( ( INDEXCOUNT - INDEXMINR ) / 2 + 1 )
#define INDEXSLIPL	( ( INDEXCOUNT - INDEXMINR ) / 2 )

		index_hdr	index[ INDEXCOUNT + 1 ];
		data_hdr	data[1];

		struct
		{
			page_index_t	logger_page_first;
			page_index_t	logger_page_last;
			page_index_t	logger_rec_max;
			page_index_t	logger_rec_cur;
			time_t		logger_chain;
			page_index_t	unused[3];
			logger_hdr	logger_head[1];
		};
	};
};
#pragma pack()

inline Page*        Layout2Page     ( void *p );
inline PageCache*   Layout2PageCache( void *p );
inline page_index_t Layout2PageIndex( void *p );
inline void* layout_ptr( void *p );
inline page_pos_t layout_offset( void *p );
inline void* extract_data( data_hdr  *hdr, size_t &size,    void *data_buf );
inline void* extract_key ( data_hdr  *hdr, size_t &key_len );
inline void* extract_key ( data_hdr  *hdr, size_t &key_len, void *key_buf  );
inline void* extract_val ( data_hdr  *hdr, size_t &val_len, void *val_buf  );
inline void* extract_val ( data_hdr  *hdr, size_t &val_len );
inline void* extract_data( index_hdr *hdr, size_t &size,    void *data_buf );
inline void* extract_key ( index_hdr *hdr, size_t &key_len );
inline void* extract_key ( index_hdr *hdr, size_t &key_len, void *key_buf  );
inline void* extract_val ( index_hdr *hdr, size_t &val_len, void *val_buf  );
inline void* extract_val ( index_hdr *hdr, size_t &val_len );
inline void  free_data   ( data_hdr  *hdr );
inline int compare( index_hdr *hdr, const void *key, size_t key_len);

class IQueryKey
{
public:
	virtual ~IQueryKey() { }
	virtual bool update( const void *key, size_t key_len ) = 0;
};

class IQueryData
{
public:
	virtual ~IQueryData() { }
	virtual bool update( const void *key, size_t key_len, const void *val, size_t val_len ) = 0;
};
/*
inline size_t pread( int fd, void *buffer, size_t size, off_t off )
{
        lseek( fd, off, SEEK_SET );
        return read( fd, buffer, size );
}

inline size_t pwrite( int fd, const void *buffer, size_t size, off_t off )
{
        lseek( fd, off, SEEK_SET );
        return write( fd, buffer, size );
}
*/
class PageFile
{
	int	fd;
	char    *ident;
	counter_t count_read, count_write, count_sync;
public:
	~PageFile() { close(fd); free(ident); }
	struct Exception { };

#ifdef O_BINARY
	PageFile ( const char *path, int flags = O_CREAT|O_RDWR|O_BINARY, mode_t mode = 0600 )
#else
	PageFile ( const char *path, int flags = O_CREAT|O_RDWR, mode_t mode = 0600 )
#endif
	{
		if ( (fd = open( path, flags, mode )) == -1 )
			throw Exception();
		count_read = count_write = count_sync = 0;
		const char *p = strrchr( path, '/' );
		ident = strdup(p ? p + 1 : path);
	}

	const char *identity() const { return ident; }

	void read( page_index_t idx, void *data )
	{
		count_read++;
		if ( pread( fd, data, PAGESIZE, (off_t)idx * PAGESIZE ) != PAGESIZE )
			throw Exception();
	}

	void write( page_index_t idx, const void *data )
	{
		count_write++;
		if ( pwrite( fd, data, PAGESIZE, (off_t)idx * PAGESIZE ) != PAGESIZE )
			throw Exception();
	}

	void truncate( page_index_t idx )
	{
		if ( ftruncate( fd, (off_t)idx * PAGESIZE ) == -1 )
			throw Exception();
	}

	void sync()
	{
		count_sync++;
		if ( fsync(fd) == -1 )
			throw Exception();
	}

	counter_t read_counter()  const { return count_read;  }
	counter_t write_counter() const { return count_write; }
	counter_t sync_counter()  const { return count_sync;  }
};

class PageMemory
{
	typedef AlignedMemoryBlock< PAGESIZE, MEMSLOTSIZE > PageMemoryType;
	static PageMemoryType* GetInstance() { static PageMemoryType mem; return &mem; }
public:
	static void *alloc() { return GetInstance()->alloc(); }
	static void free(void *p) { GetInstance()->free(p); }
};

class Page
{
	page_index_t	idx;
	Page            *next_hash_equal;
	lru_t		lru;
	PageLayout	*layout;
	PageCache	*page_cache;
	ptrdiff_t	snapshot;
	enum { CLEAN = 0, DIRTY = 1 };

	struct lruless { bool operator()(Page * p1, Page * p2) const { return p1->lru < p2->lru; } };
public:
	static void sort_lru( Page **it, Page **ie ) { std::sort( it, ie, lruless()); }

	Page( page_index_t index, PageCache *cache ) : idx(index), lru(0), 
		layout((PageLayout *)PageMemory::alloc()), page_cache(cache), snapshot((ptrdiff_t)layout) { *(Page **)(layout + 1) = this; }
	~Page() { PageMemory::free(layout); }
	bool set_snapshot()
	{
		if ( snapshot & DIRTY )
		{
			PageLayout *tmp = (PageLayout *)PageMemory::alloc();
			memcpy( tmp, layout, PAGESIZE );
			*(page_index_t *)(tmp + 1) = idx;
			snapshot = (ptrdiff_t)tmp;
			return true;
		}
		return false;
	}
	bool clr_snapshot()
	{
		PageMemory::free( (void *)(snapshot & PAGEMASK) );
		return snapshot & DIRTY;
	}
	Page* set_dirty()
	{
		snapshot |= DIRTY;
		return this;
	}
	bool is_clean() const { return (snapshot & ~PAGEMASK) == CLEAN; }
	PageLayout *layout_ptr() { return layout; }
	const PageLayout *layout_ptr() const { return layout; }
	page_index_t index() const { return idx; }
	page_index_t get_free_page_list() const { return layout->free_page_list; }
	void set_free_page_list( page_index_t idx ) { set_dirty(); layout->free_page_list = idx; }
	const PageLayout* snapshot_ptr() const { return (const PageLayout *)(snapshot & PAGEMASK); }
	PageCache* cache() { return page_cache; }
	void set_lru( lru_t l ) { lru = l; }
	Page *get_next_hash_equal() const { return next_hash_equal; }
	void set_next_hash_equal(Page *page) { next_hash_equal = page; }

	void init_magic()
	{
		memset( layout, 0, sizeof(*layout) );
		layout->root_index_idx = 1;
	}
	page_index_t get_root_index_idx() const { return layout->root_index_idx; }
	void set_root_index_idx( page_index_t idx ) { set_dirty(); layout->root_index_idx = idx; }
	page_index_t get_frag_page_index( int i ) const { return layout->frag_page_index[i]; }
	page_pos_t   get_frag_page_pos  ( int i ) const { return layout->frag_page_pos[i];   }
	void set_frag_page_position( int i, page_index_t page_index, page_pos_t page_pos )
	{
		set_dirty(); layout->frag_page_index[i] = page_index; layout->frag_page_pos[i] = page_pos;
	}
	page_index_t extend_page() { set_dirty(); return ++layout->max_page_idx; }
	Performance* performance_ptr() { return &(layout->performance); }

private:
	Performance*  performance();
	index_hdr*    index_begin()  { return layout->index + index_end()->l_pos; }
	index_hdr*    index_end()    { return layout->index + INDEXCOUNT; }
	index_hdr*    index_tail()   { return layout->index + index_end()->r_pos; }
	index_hdr*    index_pos(page_pos_t page_pos) { return layout->index + page_pos; }
	page_pos_t&   index_l_pos()  { return index_end()->l_pos; }
	page_pos_t&   index_r_pos()  { return index_end()->r_pos; }
	page_pos_t    index_m_pos()  { index_hdr *end = index_end(); return ( end->r_pos + end->l_pos ) >> 1; }
	page_pos_t    index_size()   { index_hdr *end = index_end(); return end->r_pos - end->l_pos; }
	page_pos_t    index_offset( index_hdr *pos ) { return layout_offset( pos ) / sizeof(index_hdr); }
	page_pos_t&   parent_pos()   { return index_end()->parent_pos; }
	page_index_t& parent_index() { return index_end()->parent_index; }
	static void copy_key( index_hdr *dst, index_hdr *src )
	{
		page_index_t *p = (page_index_t *)dst;
		page_index_t *q = (page_index_t *)src;
		*++p = *++q;
		*++p = *++q;
		*++p = *++q;
	}
	static void copy_key_and_left ( index_hdr *dst, index_hdr *src )
	{
		*dst = *src;
	}
	static void copy_key_and_right( index_hdr *dst, index_hdr *src )
	{
		page_index_t *p = (page_index_t *)dst;
		page_index_t *q = (page_index_t *)src;
		*++p = *++q;
		*++p = *++q;
		*++p = *++q;
		*++p = *++q;
	}
	static page_index_t& L_index( index_hdr *hdr ) { return hdr->child_index;     }
	static page_index_t& R_index( index_hdr *hdr ) { return (hdr+1)->child_index; }
	Page* RL_page( index_hdr *hdr );
	Page* WL_page( index_hdr *hdr );
	Page* RR_page( index_hdr *hdr );
	Page* WR_page( index_hdr *hdr );
	Page* RI_page( page_index_t page_index );
	Page* WI_page( page_index_t page_index );
	Page* alloc_root_index_page();
	Page* alloc_page();
	void  free_page( Page *page );
	void  set_root_index( Page *page );
	void set_parent( page_index_t parent_index, page_pos_t parent_pos )
	{
		index_hdr *hdr    = index_end();
		hdr->parent_index = parent_index;
		hdr->parent_pos   = parent_pos;
	}

	void split()
	{
		performance()->split();
		index_hdr *mid         = index_pos(INDEXMINL);
		Page *parent           = parent_index() ? WI_page(parent_index()) : alloc_root_index_page();
		Page *sibling          = alloc_page();
		index_hdr *sibling_bgn = sibling->index_begin();
		sibling->index_r_pos() = INDEXMINR;
		index_r_pos()          = INDEXMINL;
		memcpy( sibling_bgn, mid + 1, INDEXMINR * sizeof(index_hdr) + sizeof(page_index_t) );
		if ( L_index(sibling_bgn) )
		{
			page_index_t sibling_index = sibling->index();
			for ( page_pos_t i = 0; i < INDEXMINR+1; i++ )
				WL_page( sibling_bgn + i )->set_parent( sibling_index, i );
		}
		parent->insert_internal( parent->index_pos(parent_pos()), mid, this, sibling );
	}

	void insert_internal( index_hdr *pos, index_hdr *mid, Page *l_child, Page *r_child )
	{
		if ( index_l_pos() == 0 || (index_m_pos() < index_offset(pos) && index_r_pos() < INDEXCOUNT) )
		{
			performance()->insert_internal_adjust_right();
			for (index_hdr *tail = index_tail(); tail > pos; tail-- )
			{
				copy_key_and_right ( tail, tail - 1 );
				WR_page(tail)->parent_pos()++;
			}
			index_r_pos() ++;
		}
		else
		{
			performance()->insert_internal_adjust_left();
			pos--;
			for (index_hdr *bgn = index_begin() - 1; bgn < pos; bgn++ )
			{
				copy_key_and_left ( bgn, bgn + 1 );
				WL_page(bgn)->parent_pos()--;
			}
			index_l_pos() --;
		}
		copy_key( pos, mid );
		L_index( pos ) = l_child->index();
		R_index( pos ) = r_child->index();
		l_child->set_parent( index(), index_offset(pos) );
		r_child->set_parent( index(), index_offset(pos) + 1 );
		if ( index_size() == INDEXCOUNT )
			split();
	}

	void merge_sibling( index_hdr *parent_hdr, Page *sibling )
	{
		performance()->merge_sibling();
		index_hdr *sibling_bgn  = sibling->index_begin();
		page_pos_t sibling_size = sibling->index_size();
		if ( (page_pos_t)INDEXCOUNT < index_r_pos() + sibling_size + 1 )
		{
			performance()->merge_sibling_adjust();
			index_hdr *cur  = index_pos( 0 );
			index_hdr *bgn  = index_begin();
			if ( L_index(bgn) )
			{
				page_pos_t page_pos = 0;
				for ( index_hdr *tail = index_tail(); bgn < tail; cur++, bgn++ )
				{
					copy_key_and_left( cur, bgn );
					WL_page(cur)->parent_pos() = page_pos++;
				}
				WI_page( L_index(cur) = L_index(bgn) )->parent_pos() = page_pos;
			}
			else
				for ( index_hdr *tail = index_tail(); bgn < tail; copy_key( cur++, bgn++ ) );
			index_r_pos() = index_size();
			index_l_pos() = 0;
		}
		Page *parent = Layout2Page(parent_hdr);
		index_hdr *tail   = index_tail();
		copy_key( tail, parent_hdr );
		memcpy( tail + 1, sibling_bgn, sibling_size * sizeof(index_hdr) + sizeof(page_index_t) );
		index_r_pos() += sibling_size + 1;
		free_page( sibling );
		if ( R_index(tail) )
		{
			page_index_t page_index = index();
			page_pos_t   page_pos   = index_offset(tail) + 1;
			for ( index_hdr *new_tail = index_tail(); tail < new_tail; tail ++ )
				WR_page(tail)->set_parent( page_index, page_pos++ );
		}
		if ( parent->index_m_pos() < index_offset(parent_hdr) )
		{
			for ( index_hdr *parent_tail = parent->index_tail() - 1; parent_hdr < parent_tail; parent_hdr ++ )
			{
				copy_key_and_right( parent_hdr, parent_hdr + 1 );
				WR_page(parent_hdr)->parent_pos() --;
			}
			parent->index_r_pos() --;
		}
		else
		{
			for ( index_hdr *parent_bgn = parent->index_begin(); parent_hdr > parent_bgn; parent_hdr -- )
			{
				copy_key_and_right( parent_hdr, parent_hdr - 1 );
				WR_page(parent_hdr)->parent_pos() ++;
			}
			WI_page( R_index(parent_hdr) = L_index(parent_hdr) )->parent_pos() ++;
			parent->index_l_pos() ++;
		}
		if ( parent->index_size() < INDEXMINR )
		{
			if ( page_index_t ancestry_index = parent->parent_index() )
			{
				performance()->merge_sibling_nest();
				Page *ancestry = WI_page( ancestry_index );
				if ( ancestry->index_r_pos() == parent->parent_pos() )
					parent->l_shrink_internal( ancestry->index_pos(parent->parent_pos()) - 1 );
				else
					parent->r_shrink_internal( ancestry->index_pos(parent->parent_pos()) );
			}
			else
			{
				if ( parent->index_size() == 0 )
				{
					set_parent( 0, 0 );
					set_root_index(this);
					free_page(parent);
				}
			}
		}
	}

	void r_shrink_internal( index_hdr *parent_hdr )
	{
		performance()->r_shrink_internal();
		Page *sibling = WR_page( parent_hdr );

		if ( sibling->index_size() > INDEXMINR )
		{
			index_hdr *sibling_bgn = sibling->index_begin();
			index_hdr *tail        = index_tail();
			if ( index_r_pos() == INDEXCOUNT )
			{
				performance()->r_shrink_internal_adjust();
				index_hdr *cur = index_pos( INDEXSLIPL );
				index_hdr *bgn = index_begin();
				for ( ;bgn < tail; bgn++, cur++ )
				{
					copy_key_and_left( cur, bgn );
					WL_page(cur)->parent_pos() = index_offset(cur);
				}
				WI_page( L_index(cur) = L_index(bgn) )->parent_pos() = index_offset(cur);
				index_l_pos() = INDEXSLIPL;
				index_r_pos() = INDEXSLIPL + INDEXMINR - 1;
				tail = index_tail();
			}
			copy_key( tail, parent_hdr );
			copy_key( parent_hdr, sibling_bgn );
			WI_page( R_index(tail) = L_index( sibling_bgn ) )->set_parent( index(), index_offset(tail) + 1 ); 
			index_r_pos() ++;
			sibling->index_l_pos() ++;
		}
		else
			merge_sibling( parent_hdr, sibling );
	}

	void r_shrink_leaf( index_hdr *parent_hdr )
	{
		performance()->r_shrink_leaf();
		Page *sibling = WR_page( parent_hdr );

		if ( sibling->index_size() > INDEXMINR )
		{
			index_hdr *sibling_bgn = sibling->index_begin();
			index_hdr *tail        = index_tail();
			if ( index_r_pos() == INDEXCOUNT )
			{
				performance()->r_shrink_leaf_adjust();
				for ( index_hdr *cur = index_pos(INDEXSLIPL), *bgn = index_begin(); bgn < tail; copy_key( cur++, bgn++ ) );
				index_l_pos() = INDEXSLIPL;
				index_r_pos() = INDEXSLIPL + INDEXMINR - 1;
				tail = index_tail();
			}
			copy_key( tail, parent_hdr );
			copy_key( parent_hdr, sibling_bgn );
			index_r_pos() ++;
			sibling->index_l_pos() ++;
		}
		else
			merge_sibling( parent_hdr, sibling );
	}

	void l_shrink_internal( index_hdr *parent_hdr )
	{
		performance()->l_shrink_internal();
		Page *sibling = WL_page(parent_hdr);

		if ( sibling->index_size() > INDEXMINR )
		{
			if ( index_l_pos() == 0 )
			{
				performance()->l_shrink_internal_adjust();
				index_hdr *tail = index_tail();
				index_hdr *cur  = tail + INDEXSLIPR;
				for ( index_hdr *bgn = index_begin(); tail > bgn; )
				{
					copy_key_and_right( --cur, --tail );
					WR_page(cur)->parent_pos() += INDEXSLIPR;
				}
				WI_page( L_index(cur) = L_index(tail) )->parent_pos() += INDEXSLIPR;
				index_l_pos() = INDEXSLIPR - 1;
				index_r_pos() = INDEXSLIPR + INDEXMINR - 1;
			}
			else
				index_l_pos() --;
			sibling->index_r_pos() --;
			index_hdr *sibling_tail = sibling->index_tail();
			index_hdr *bgn          = index_begin();
			copy_key( bgn, parent_hdr );
			copy_key( parent_hdr, sibling_tail );
			WI_page( L_index(bgn) = R_index( sibling_tail ) )->set_parent( index(), index_offset(bgn) );
		}
		else
			sibling->merge_sibling( parent_hdr, this );
	}

	void l_shrink_leaf( index_hdr *parent_hdr )
	{
		performance()->l_shrink_leaf();
		Page *sibling = WL_page(parent_hdr);

		if ( sibling->index_size() > INDEXMINR )
		{
			if ( index_l_pos() == 0 )
			{
				performance()->l_shrink_leaf_adjust();
				for( index_hdr *tail = index_tail(), *bgn = index_begin(), *cur = tail + INDEXSLIPR; tail > bgn; copy_key( --cur, --tail ));
				index_l_pos() = INDEXSLIPR - 1;
				index_r_pos() = INDEXSLIPR + INDEXMINR - 1;
			}
			else
				index_l_pos() --;
			copy_key( index_begin(), parent_hdr );
			sibling->index_r_pos() --;
			copy_key( parent_hdr, sibling->index_tail() );
		}
		else
			sibling->merge_sibling( parent_hdr, this );
	}

	void remove_internal( index_hdr *pos )
	{
		set_dirty();
		performance()->remove_internal();
		index_hdr *right_most;
		for ( right_most = RL_page(pos)->index_tail() - 1; R_index( right_most ); right_most = RR_page(right_most)->index_tail() - 1 );
		copy_key( pos, right_most );
		Layout2Page(right_most)->remove_leaf( right_most );
	}

	void remove_leaf( index_hdr *pos )
	{
		set_dirty();
		performance()->remove_leaf();
		if ( index_m_pos() < index_offset(pos) )
		{
			for ( index_hdr *tail = index_tail() - 1; pos < tail; pos++ )
				copy_key( pos, pos + 1 );
			index_r_pos() --;
		}
		else
		{
			for ( index_hdr *bgn = index_begin(); pos > bgn; pos-- )
				copy_key( pos, pos - 1 );
			index_l_pos() ++;
		}
		if ( index_size() < INDEXMINR && parent_index() )
		{
			Page *parent     = WI_page(parent_index());
			index_hdr *parent_hdr = parent->index_pos(parent_pos());
			if ( parent->index_r_pos() == parent_pos() )
				l_shrink_leaf( parent_hdr - 1 );
			else
				r_shrink_leaf( parent_hdr );
		}
	}

public:
	void remove( index_hdr *pos )
	{
		if ( L_index(pos) )
			remove_internal(pos);
		else
			remove_leaf(pos);
	}

	index_hdr* find_key( const void *key, size_t key_len, int& pos )
	{
		performance()->find_key();
		if ( size_t size = index_size() )
			for ( Page *page = this; ; size = page->index_size() )
			{
				index_hdr *hdr = page->index_begin();
				do
				{
					int half = size >> 1;
					pos = compare( hdr + half, key, key_len );
					if ( pos > 0 )
					{
						size -= half + 1;
						hdr  += half + 1;
					}
					else if ( pos < 0 )
						size  = half;
					else
						return hdr + half;
				} while ( size );
				if ( page_index_t page_index = L_index(hdr) )
					page = RI_page(page_index);
				else
					return hdr;
			}
		return NULL;
	}

	index_hdr* find( const void *key, size_t key_len )
	{
		int pos;
		index_hdr *hdr = find_key( key, key_len, pos );
		return ( hdr && !pos ) ? hdr : NULL;
	}

	index_hdr* great_equal_key( const void *key, size_t key_len )
	{
		int pos;
		index_hdr* hdr=find_key( key, key_len, pos ); 
		return (hdr && hdr==Layout2Page(hdr)->index_tail()) ? (Layout2Page(hdr))->next(hdr-1): hdr;
	}

	index_hdr* insert_init_node()
	{
		set_dirty();
		index_l_pos()  = INDEXMINR;
		index_r_pos()  = INDEXMINR + 1;
		index_hdr *bgn = index_begin();
		L_index(bgn)   = R_index(bgn) = 0;
		return bgn;
	}

	void insert_leaf( index_hdr *pos, index_hdr *val )
	{
		set_dirty();
		if ( index_l_pos() == 0 || (index_m_pos() < index_offset(pos) && index_r_pos() < INDEXCOUNT) )
		{
			performance()->insert_leaf_adjust_right();
			for ( index_hdr *tail = index_tail(); tail > pos; tail-- )
				copy_key( tail, tail - 1 );
			index_r_pos() ++;
		}
		else
		{
			performance()->insert_leaf_adjust_left();
			pos --;
			for (index_hdr *bgn = index_begin() - 1; bgn < pos; bgn++ )
				copy_key( bgn, bgn + 1 );
			index_l_pos() --;
		}
		copy_key( pos, val );
		L_index(pos) = R_index(pos) = 0;
		if ( index_size() == INDEXCOUNT )
			split();
	}

	index_hdr* left_most()
	{
		index_hdr *bgn = index_begin();
		while ( L_index(bgn) )
			bgn = RL_page(bgn)->index_begin();
		index_hdr *left =  bgn < Layout2Page(bgn)->index_tail() ? bgn : NULL;
		return left;
	}

	index_hdr* next(index_hdr *cur)
	{
		if ( R_index(cur) )
			return RR_page(cur)->left_most();
		if ( ++cur < index_tail() )
			return cur;
		for ( index_hdr *end = index_end(); end->parent_index; )
		{
			Page *parent = RI_page(end->parent_index);
			if ( end->parent_pos < parent->index_r_pos() )
				return parent->index_pos( end->parent_pos );
			end = parent->index_end();
		}
		return NULL;
	}

	static void* operator new(size_t size);
	static void operator delete(void *p);
};

class PagePool
{
	enum { AlignedSize = ( sizeof(Page) + 3 ) & ~3 };
	typedef AlignedMemoryBlock< AlignedSize, PAGEPOOLSIZE, DefaultMemoryAllocator > PagePoolType;
	static PagePoolType* GetInstance() { static PagePoolType pool; return &pool; }
public:
	static void *alloc() { return GetInstance()->alloc(); }
	static void free(void *p) { return GetInstance()->free(p); }
};

inline void* Page::operator new(size_t size) { return PagePool::alloc(); }
inline void Page::operator delete(void *p) { return PagePool::free(p); }

class PageHash
{
	DefaultMemoryAllocator allocator;
	lru_t	lru;
	size_t	_size;
	size_t  bucket_mask;
	size_t  snapshot_size, snapshot_capacity;
	size_t	cache_high;
	size_t	cache_low;
	Page**  bucket;
	Page**  snapshot;

	void erase( Page *page )
	{
		Page **it = bucket + (page->index() & bucket_mask);
		if ( page == *it )
			*it = page->get_next_hash_equal();
		else for ( Page *next_next, *next = *it; next; next = next_next )
			if ( (next_next = next->get_next_hash_equal() ) == page )
			{
				next->set_next_hash_equal( page->get_next_hash_equal() );
				break;
			}
		_size --;
		delete page;
	}

	Page* lru_adjust( Page *page )
	{
		page->set_lru(lru);
		if ( ++lru == 0 ) lru_adjust();
		return page;
	}

	void lru_adjust()
	{
		Page **tmp = (Page **)allocator.alloc(_size * sizeof(Page *)), **it = tmp, **ii = tmp;
		for ( Page** it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it )
			for ( Page *page = *it ; page ; page = page->get_next_hash_equal() )
				*ii++ = page;
		Page::sort_lru( it, ii );
		for ( lru = 0; it != ii; ++it )
			(*it)->set_lru(lru++);
		allocator.free(tmp, _size * sizeof(Page *));
	}

public:
	void cleanup()
	{
		if ( _size <= cache_high )
			return;
		size_t tmp_size = _size;
		Page **tmp = (Page **)allocator.alloc(tmp_size * sizeof(Page *)), **it = tmp, **ii = tmp;
		for ( Page** it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it )
			for ( Page *page = *it ; page ; page = page->get_next_hash_equal() )
				*ii++ = page;
		Page::sort_lru( it, ii );
		for ( lru = 0; _size > cache_low && it != ii; ++it )
			if ( (*it)->is_clean() )
				erase( *it );
			else
				(*it)->set_lru(lru++);
		for ( ; it != ii; ++it )
			(*it)->set_lru(lru++);
		allocator.free(tmp, tmp_size * sizeof(Page *));
	}

	~PageHash()
	{
		clear();
		allocator.free(bucket, (bucket_mask + 1) * sizeof(Page *) );
		allocator.free(snapshot, snapshot_capacity * sizeof(Page *) );
	}

	PageHash() : lru(0), _size(0), bucket_mask(15), snapshot_size(0), snapshot_capacity(0), bucket(NULL), snapshot(NULL) { }

	void init( size_t high, size_t low )
	{
		size_t bucket_size = bucket_mask + 1;
		cache_high = high; cache_low = low;
		while ( bucket_size < cache_high ) bucket_size <<= 1;
		bucket = (Page **)allocator.alloc( bucket_size * sizeof(Page *) );
		bucket_mask = bucket_size - 1;
		memset( bucket, 0, bucket_size * sizeof(Page *));
	}

	size_t size() const { return _size; }

	Page** snapshot_reference(size_t& size) { size = snapshot_size; return snapshot; }

	size_t snapshot_create( Page *magic )
	{
		if ( snapshot_capacity < _size + 1 )
		{
			allocator.free(snapshot, snapshot_capacity * sizeof(Page *) );
			snapshot = (Page **)allocator.alloc( (snapshot_capacity = _size + 1) * sizeof(Page *) );
		}
		snapshot_size = 0;
		for ( Page **it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it )
			for ( Page *page = *it ; page ; page = page->get_next_hash_equal() )
				if ( page->set_snapshot() )
					snapshot[snapshot_size++] = page;
		if ( snapshot_size )
		{
			magic->set_dirty()->set_snapshot();
			snapshot[snapshot_size++] = magic;
		}
		else if ( magic->set_snapshot() )
			snapshot[snapshot_size++] = magic;
		return snapshot_size;
	}

	size_t snapshot_release()
	{
		size_t dirty_count = 0;
		for ( Page **it = snapshot, **ie = snapshot + snapshot_size; it != ie; ++it )
			if ( (*it)->clr_snapshot() )
				dirty_count++;
		cleanup();
		return dirty_count;
	}

	Page* insert( Page *page )
	{
		Page** it = bucket + (page->index() & bucket_mask);
		page->set_next_hash_equal(*it);
		*it = page;
		if ( _size++ == bucket_mask )
		{
			size_t tmp_mask = ( (bucket_mask + 1) << 1 ) - 1;
			Page** tmp = (Page **)allocator.alloc( (tmp_mask + 1) * sizeof(Page *) );
			memset( tmp, 0, (tmp_mask + 1) * sizeof(Page *));
			for ( Page **it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it )
				for ( Page *next, *page = *it ; page ; page = next )
				{
					next = page->get_next_hash_equal();
					Page** it = tmp + (page->index() & tmp_mask);
					page->set_next_hash_equal( *it );
					*it = page;
				}
			allocator.free(bucket, (bucket_mask + 1) * sizeof(Page *) );
			bucket      = tmp;
			bucket_mask = tmp_mask;
		}
		return lru_adjust( page );
	}

	Page* find( page_index_t page_index )
	{
		for ( Page *page = bucket[page_index & bucket_mask]; page; page = page->get_next_hash_equal() )
			if ( page->index() == page_index )
				return lru_adjust(page);
		return NULL;
	}

	void clear()
	{
		for ( Page **it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it )
			if ( Page *page = *it )
			{
				*it = NULL;
				Page *next;
				do
				{
					next = page->get_next_hash_equal();
					delete page;
				}
				while ( (page = next) );
			}
		_size = 0;
	}
};

class PageCache
{
	PageFile    *page_file;
	Page         magic;
	PageHash     hash;
public:
	PageCache ( PageFile *file, size_t cache_high, size_t cache_low ) : page_file(file), magic(0, this)
	{
		try { page_file->read( 0, magic.layout_ptr() ); *(Page **)(magic.layout_ptr() + 1) = &magic; }
		catch ( PageFile::Exception e ) { magic.init_magic(); }
		magic.set_dirty();
		Performance *perf = performance();
		if ( cache_high && cache_low )
		{
			perf->set_cache_high( cache_high );
			perf->set_cache_low ( cache_low  );
		}
		hash.init( perf->get_cache_high(), perf->get_cache_low() );
		perf->reset_peak();
		perf->set_page_read(0);
		perf->set_page_write(0);
		perf->set_page_sync(0);
		try { Rload( magic.get_root_index_idx() );  }
		catch ( PageFile::Exception e ) { set_root_index(alloc()); }
	}

	PageLayout *magic_layout_ptr() { return magic.layout_ptr(); }
	Performance* performance() { return magic.performance_ptr(); }

	Page** snapshot_reference(size_t &snapshot_size) { return hash.snapshot_reference(snapshot_size); }
	void snapshot_create()
	{
		performance()->set_dirty_peak( hash.snapshot_create(&magic) );
	}

	void snapshot_release()
	{
		Performance *perf = performance();
		perf->set_page_read ( page_file->read_counter()  );
		perf->set_page_write( page_file->write_counter() );
		perf->set_page_sync ( page_file->sync_counter()  );
		perf->set_cache_peak( hash.size() );
		perf->set_snapshot_peak( hash.snapshot_release() );
	}

	Page *load_cache( page_index_t idx )
	{
		Page *page = new Page( idx, this );
		try
		{
			page_file->read( idx, page->layout_ptr() );
			*(Page **)(page->layout_ptr() + 1) = page;
		}
		catch ( ... ) { delete page; throw; }
		return hash.insert( page );
	}

	Page *__load( page_index_t idx )
	{
		if ( Page *page = hash.find( idx ) )
		{
			performance()->page_load_hit();
			return page;
		}
		performance()->page_load_miss();
		return load_cache(idx);
	}

	Page *Rload( page_index_t idx ) { return __load(idx); }
	Page *Wload( page_index_t idx ) { return __load(idx)->set_dirty(); }
	void *Rload_hdr( page_index_t idx, page_pos_t pos ) { return (byte_t *)(Rload(idx)->layout_ptr()) + pos; }
	void *Wload_hdr( page_index_t idx, page_pos_t pos ) { return (byte_t *)(Wload(idx)->layout_ptr()) + pos; }

	Page *alloc()
	{
		performance()->page_alloc();
		Page *page;
		if ( page_index_t idx = magic.get_free_page_list() )
		{
			if ( (page = hash.find(idx)) == NULL )
				page = load_cache(idx);
			magic.set_free_page_list(page->get_free_page_list());
		}
		else
			page = hash.insert( new Page( magic.extend_page(), this ) );
		memset( page->layout_ptr(), 0, sizeof(*page->layout_ptr()) );
		return page->set_dirty();
	}

	void free( Page *page )
	{
		performance()->page_free();
		page->set_free_page_list( magic.get_free_page_list() );
		magic.set_free_page_list( page->index() );
	}

	Page *root_index_page() { return Rload(magic.get_root_index_idx()); }
	Page *alloc_root_index_page()
	{
		Page *page = alloc();
		magic.set_root_index_idx( page->index() );
		return page;
	}
	void set_root_index( Page *page ) { magic.set_root_index_idx(page->index()); }

	void clr_fragment( frag_hdr *hdr )
	{
		if ( hdr->prev_page_index )
		{
			frag_hdr *frag_prev = hdr->Wprev_page_head();
			if ( hdr->next_page_index )
			{
				frag_hdr *frag_next = hdr->Wnext_page_head();
				frag_prev->set_next(Layout2PageIndex(frag_next), layout_offset(frag_next));
				frag_next->set_prev(Layout2PageIndex(frag_prev), layout_offset(frag_prev));
			}
			else
				frag_prev->set_next( 0, 0 );
		}
		else
		{
			int i = ((data_hdr *)hdr)->capacity() / FRAGMIN;
			if ( hdr->next_page_index )
			{
				frag_hdr *frag_next = hdr->Wnext_page_head();
				magic.set_frag_page_position(i, Layout2PageIndex(frag_next), layout_offset(frag_next));
				frag_next->set_prev( 0, 0 );
			}
			else
				magic.set_frag_page_position( i, 0, 0 );
		}
	}

	void set_fragment( frag_hdr *hdr )
	{
		page_pos_t capacity = ((data_hdr *)hdr)->capacity();
		if ( capacity == PAGEMAXSPARE )
		{
			free( Layout2Page(hdr) );
			return;
		}
		int i = capacity / FRAGMIN;
		page_index_t next_page_index = magic.get_frag_page_index(i);
		page_index_t curr_page_index = Layout2PageIndex(hdr);
		page_pos_t   next_page_pos   = magic.get_frag_page_pos(i);
		page_pos_t   curr_page_pos   = layout_offset(hdr);
		if ( next_page_index )
			((frag_hdr*)Wload_hdr( next_page_index, next_page_pos ))->set_prev( curr_page_index, curr_page_pos );
		hdr->set_next( next_page_index, next_page_pos );
		hdr->set_prev( 0, 0 );
		magic.set_frag_page_position(i, curr_page_index, curr_page_pos);
	}

	data_hdr* __alloc_data_head( size_t size )
	{
		if ( size < PAGEMAXSPARE )
			for ( int i = (size + FRAGMIN - 1) / FRAGMIN; i < PAGEUSED / FRAGMIN; i++ )
				if ( page_index_t next_page_index = magic.get_frag_page_index(i) )
				{
					frag_hdr *hdr = (frag_hdr *)Wload_hdr(next_page_index, magic.get_frag_page_pos(i));
					magic.set_frag_page_position(i, next_page_index = hdr->next_page_index, hdr->next_page_pos);
					if ( next_page_index )
						((frag_hdr *)Wload_hdr(next_page_index, hdr->next_page_pos))->set_prev( 0, 0 );
					return (data_hdr *)hdr;
				}
		data_hdr *hdr = alloc()->layout_ptr()->data;
		hdr->next     = PAGEMAXSPARE + sizeof(data_hdr);
		return hdr;
	}

	void adjust_data_head( data_hdr *hdr, size_t size )
	{
		if ( hdr->capacity() >= PAGEMINSPARE + size + sizeof( data_hdr ) )
		{
			data_hdr *ndr = (data_hdr *)((ptrdiff_t)((byte_t *)(hdr + 1) + size + 15) & ~(ptrdiff_t)15);
			data_hdr *next = hdr->next_head();
			if ( next && next->size == 0 )
			{
				clr_fragment( (frag_hdr *)next );
				ndr->next = next->next;
			}
			else
				ndr->next = hdr->next;
			//ndr->next = ( ( next && next->size == 0) ? next : hdr ) -> next;
			
			ndr->size = 0;
			set_fragment( (frag_hdr *)ndr );
			hdr->next = layout_offset( ndr );
		}
	}

	data_hdr* alloc_data_head( size_t size )
	{
		data_hdr *hdr = __alloc_data_head(size);
		if ( size < PAGEMAXSPARE )
		{
			hdr->size = size;
			adjust_data_head( hdr, size );
		}
		else
			hdr->size = PAGEMAXSPARE;
		hdr->first_slice = 1;
		return hdr;
	}

	data_hdr* alloc_data_head( data_hdr *pdr, size_t size )
	{
		data_hdr *hdr = alloc_data_head( size );
		hdr->first_slice = 0;
		pdr->set_next( Layout2PageIndex(hdr), layout_offset(hdr) );
		return hdr;
	}

	void create_node( index_hdr& index, const void *key, size_t key_len, const void *val, size_t val_len )
	{
		if ( key_len > sizeof(index.key) )
		{
			memcpy( index.key, key, sizeof(index.key) );
			index.key_len = 0;
		}
		else
		{
			memcpy( index.key, key, key_len );
			index.key_len = key_len;
		}
		size_t size      = key_len + val_len + sizeof(size32_t) + sizeof(size32_t);
		data_hdr *hdr    = alloc_data_head( size );
		size_t capacity  = hdr->size;
		byte_t *p        = (byte_t *)(hdr + 1);
		index.set_data_position(Layout2PageIndex(hdr), layout_offset(hdr));
		*(size32_t *)p   = size;    p += sizeof(size32_t);
		*(size32_t *)p   = key_len; p += sizeof(size32_t);
		if ( capacity < size )
		{
			size     -= sizeof(size32_t) + sizeof(size32_t);
			capacity -= sizeof(size32_t) + sizeof(size32_t);
			while ( capacity < key_len )
			{
				memcpy( p, key, capacity );
				key      = (byte_t *)key + capacity;
				key_len -= capacity;
				size    -= capacity;
				hdr      = alloc_data_head( hdr, size );
				capacity = hdr->size;
				p        = (byte_t *)(hdr + 1);
			}
			memcpy( p, key, key_len );
			p        += key_len;
			capacity -= key_len;
			while ( capacity < val_len )
			{
				memcpy( p, val, capacity );
				val      = (byte_t *)val + capacity;
				val_len -= capacity;
				hdr      = alloc_data_head( hdr, val_len );
				capacity = hdr->size;
				p        = (byte_t *)(hdr + 1);
			}
		}
		else
		{
			memcpy( p, key, key_len ); p += key_len;
		}
		memcpy( p, val, val_len );
		hdr->set_next( 0, 0 );
	}

	index_hdr* put_new_node( const void *key, size_t key_len, const void *val, size_t val_len )
	{
		int pos;
		if ( index_hdr *found = root_index_page()->find_key( key, key_len, pos ) )
		{
			if ( !pos )
				return found;
			index_hdr index;
			create_node( index, key, key_len, val, val_len );
			Layout2Page(found)->insert_leaf(found, &index);
		}
		else
			create_node( *root_index_page()->insert_init_node(), key, key_len, val, val_len );
		performance()->insert();
		return NULL;
	}

	void* put_replace( index_hdr* found, const void *key, size_t key_len, const void *val, size_t& save_val_len, bool need_origin )
	{
		data_hdr *hdr        = found->Windex_hdr2data_hdr();
		size_t val_len       = save_val_len;
		void*  origin_val    = need_origin ? extract_val( hdr, save_val_len ) : NULL;
		size_t pass_len      = key_len + sizeof(size32_t) + sizeof(size32_t);
		size_t size          = pass_len + val_len;
		size_t capacity      = hdr->capacity();
		if ( (size <= PAGEMAXSPARE && capacity < size ) || (size > PAGEMAXSPARE && capacity < PAGEMAXSPARE ))
		{
			Layout2Page(found)->set_dirty();
			free_data(hdr);
			create_node( *found, key, key_len, val, val_len );
			return origin_val;
		}	
		*(size32_t *)(hdr + 1) = size;
		for ( ;pass_len > hdr->size; hdr = hdr->Rnext_page_head() ) pass_len -= hdr->size;
		Layout2Page( hdr )->set_dirty();
		byte_t *p            = (byte_t *)(hdr + 1) + pass_len;
		data_hdr *ndr        = NULL;
		capacity             = hdr->capacity() - pass_len;
		if ( val_len <= capacity )
		{
			memcpy( p, val, val_len );
			hdr->size = pass_len + val_len;
			if ( hdr->next_page_index ) ndr = hdr->Wnext_page_head();
			adjust_data_head( hdr, hdr->size );
		}
		else
		{
			memcpy( p, val, capacity );
			hdr->size = pass_len + capacity;
			val       = (byte_t *)val + capacity;
			val_len  -= capacity;
			while ( hdr->next_page_index && (capacity = (ndr = hdr->Wnext_page_head())->size) < val_len && capacity == PAGEMAXSPARE )
			{
				hdr       = ndr;
				memcpy( hdr + 1, val, capacity );
				val       = (byte_t *)val + capacity;
				val_len  -= capacity;
			}
			if ( hdr->next_page_index == 0 ) ndr = NULL;
			if ( ndr && (capacity = ndr->capacity()) >= val_len )
			{
				hdr       = ndr;
				ndr       = hdr->next_page_index ? hdr->Wnext_page_head() : NULL;
				memcpy( hdr + 1, val, val_len );
				hdr->size = val_len;
				adjust_data_head( hdr, val_len );
			}
			else while ( val_len )
			{
				hdr       = alloc_data_head( hdr, val_len );
				capacity  = hdr->size;
				memcpy( hdr + 1, val, capacity );
				val       = (byte_t *)val + capacity;
				val_len  -= capacity;
			}
		}
		if ( ndr ) free_data(ndr);
		hdr->set_next( 0, 0 );
		performance()->insert_replace();
		return origin_val;
	}

	bool put( const void *key, size_t key_len, const void *val, size_t val_len, bool replace )
	{
		if ( index_hdr *found = put_new_node( key, key_len, val, val_len ) )
		{
			if ( replace )
				put_replace( found, key, key_len, val, val_len, false );
			else
			{
				performance()->insert_reject();
				return false;
			}
		}
		return true; 
	}

	void* put( const void *key, size_t key_len, const void *val, size_t& val_len )
	{
		if ( index_hdr *found = put_new_node( key, key_len, val, val_len ) )
			return put_replace( found, key, key_len, val, val_len, true );
		return NULL; 
	}

	void* find( const void *key, size_t key_len, size_t &val_len )
	{
		index_hdr *hdr = root_index_page()->find( key, key_len );
		return hdr ? extract_val(hdr->Rindex_hdr2data_hdr(), val_len) : NULL;
	}

	void* find( const void *key, size_t key_len, size_t &val_len, void *val_buf )
	{
		index_hdr *hdr = root_index_page()->find( key, key_len );
		return hdr ? extract_val(hdr->Rindex_hdr2data_hdr(), val_len, val_buf) : NULL;
	}

	void* first_key( size_t &key_len )
	{
		index_hdr *hdr = root_index_page()->left_most();
		return hdr ? extract_key(hdr->Rindex_hdr2data_hdr(), key_len) : NULL;
	}

	void* next_key( const void *key, size_t &key_len )
	{
		int pos;
		if (index_hdr *hdr = root_index_page()->find_key( key, key_len, pos ))
			return (hdr = Layout2Page(hdr)->next(hdr)) ? extract_key(hdr->Rindex_hdr2data_hdr(), key_len) : NULL;
		return NULL;
	}

	void walk( index_hdr *hdr, IQueryKey *query )
	{
		page_index_t lastPageIndex = 0;
		byte_t key_buf[STACKBUFFER];
		for ( bool r = true; r && hdr; hdr = Layout2Page(hdr)->next(hdr) )
		{
			page_index_t curPageIndex = Layout2Page(hdr)->index();
			if ( curPageIndex != lastPageIndex )
			{
				lastPageIndex = curPageIndex;
				hash.cleanup();
			}
			size_t key_len = sizeof(key_buf);
			void*  key     = extract_key( hdr, key_len, key_buf );
			r = query->update( key, key_len );
			if ( key != key_buf ) ::free(key);
		}
	}

	void walk( index_hdr *hdr, IQueryData *query )
	{
		page_index_t lastPageIndex = 0;
		byte_t data_buf[STACKBUFFER];
		for ( bool r = true; r && hdr; hdr = Layout2Page(hdr)->next(hdr) )
		{
			page_index_t curPageIndex = Layout2Page(hdr)->index();
			if ( curPageIndex != lastPageIndex )
			{
				lastPageIndex = curPageIndex;
				hash.cleanup();
			}
			size_t data_len = sizeof(data_buf);
			void*  data     = extract_data( hdr, data_len, data_buf );
			size_t key_len  = *((size32_t *)data + 1);
			size_t val_len  = *(size32_t *)data - key_len - sizeof(size32_t) - sizeof(size32_t);
			if ( key_len==0 ) 
			{
				if ( data != data_buf ) 
					::free(data);
				continue;
			}
			void*  key      = (byte_t *)data + sizeof(size32_t) + sizeof(size32_t);
			void*  val      = (byte_t *)key + key_len;
			r = query->update( key, key_len, val, val_len );
			if ( data != data_buf ) ::free(data);
		}
	}

	template<typename T>
	void walk( T *query ) { walk ( root_index_page()->left_most(), query ); }
	template<typename T>
	void walk( const void *key, size_t key_len, T *query ) { walk ( root_index_page()->great_equal_key( key, key_len ), query ); }

	bool del( const void *key, size_t key_len )
	{
		if ( index_hdr *hdr = root_index_page()->find( key, key_len ) )
		{
			performance()->remove_found();
			free_data( hdr->Windex_hdr2data_hdr() );
			Layout2Page(hdr)->remove( hdr );
			return true;
		}
		performance()->remove_not_found();
		return false;
	}

	void* del( const void *key, size_t key_len, size_t& val_len )
	{
		if ( index_hdr *hdr = root_index_page()->find( key, key_len ) )
		{
			performance()->remove_found();
			data_hdr *origin_hdr = hdr->Windex_hdr2data_hdr();
			void     *origin_val = extract_val( origin_hdr, val_len );
			free_data( origin_hdr );
			Layout2Page(hdr)->remove( hdr );
			return origin_val;
		}
		performance()->remove_not_found();
		return NULL;
	}

	bool exist( const void *key, size_t key_len ) { return root_index_page()->find( key, key_len ); }
	size_t record_count() { return performance()->record_count(); }
};

class Logger
{
public:
	enum State { CLEAN, PREPARED, REDO, CORRUPT };
	virtual ~Logger() { }
	virtual void  init( PageFile *file, PageCache *cache ) = 0;
	virtual State integrity_verify() { return CLEAN; }
	virtual State truncate() { return CLEAN; }
	virtual State redo(time_t timestamp ) { return CLEAN; }
	virtual State prepare() { return PREPARED; }
	virtual State commit(time_t timestamp ) = 0;
	virtual State abort() { return CLEAN; }

	static void save_page( const Page *page, PageFile *log_file, page_index_t page_index )
	{
		const PageLayout *snapshot = page->snapshot_ptr();
		log_file->write( page_index, snapshot );
	}

	template< typename Iterator >
	static void save_page( Iterator it, Iterator ie, PageFile *log_file )
	{
		for ( ; it != ie; ++it )
			save_page( *it, log_file, (*it)->index() );
	}

	template< typename Iterator >
	static page_index_t save_page( Iterator it, Iterator ie, PageFile *log_file, page_index_t page_index )
	{
		for ( ; it != ie; ++it )
			save_page( *it, log_file, page_index++ );
		return page_index;
	}

	static void restore_page( PageFile *log_file, page_index_t it, page_index_t ie, PageFile *page_file )
	{
		PageLayout *layout = (PageLayout *)PageMemory::alloc();
		for ( ; it != ie; ++it )
		{
			log_file->read( it, layout );
			page_file->write( *(page_index_t *)(layout + 1), layout );
		}
		PageMemory::free( layout );
	}
};

class NullLogger : public Logger
{
	PageFile  *page_file;
	PageCache *page_cache;
public:
	void init( PageFile *file, PageCache *cache )
	{
		page_file  = file;
		page_cache = cache;
	}
	State commit(time_t timestamp )
	{
		PageLayout *new_magic = page_cache->magic_layout_ptr();
		size_t snapshot_size;
		Page** snapshot = page_cache->snapshot_reference( snapshot_size );
		try
		{
			new_magic->logger_last_check = timestamp;
			new_magic->logger_first_idx  = 0;
			new_magic->logger_magic_idx  = 0;
			new_magic->logger_id         = 0;
			new_magic->logger_serial     = 0;
			save_page( snapshot, snapshot + snapshot_size, page_file );
			page_file->sync();
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}
};

class PageLogger : public Logger
{
	PageFile  *page_file;
	PageCache *page_cache;
	void *magic_page;
	PageLayout *org_magic;
public:
	~PageLogger() { PageMemory::free(magic_page); }
	PageLogger() : magic_page( PageMemory::alloc() ), org_magic((PageLayout *)magic_page) { memset(magic_page, 0, PAGESIZE); }

	void init( PageFile *file, PageCache *cache )
	{
		page_file  = file;
		page_cache = cache;
	}

	State integrity_verify()
	{
		State state = CLEAN;
		PageLayout *new_magic = page_cache->magic_layout_ptr();
		if ( page_index_t logger_magic_idx = new_magic->logger_magic_idx )
			try
			{
				page_file->read( logger_magic_idx, org_magic );
				state = (org_magic->logger_serial != new_magic->logger_serial) ? CORRUPT : REDO;
			}
			catch ( PageFile::Exception e )
			{
				state = CORRUPT;
			}
		memcpy(org_magic, new_magic, PAGEUSED);
		return state;
	}

	State truncate() { return abort(); }
	State redo( time_t timestamp ) { return commit(timestamp); }

	State prepare( )
	{
		try
		{
			size_t snapshot_size;
			Page** snapshot             = page_cache->snapshot_reference( snapshot_size );
			PageLayout *new_magic       = page_cache->magic_layout_ptr();
			page_index_t page_index     = std::max( new_magic->max_page_idx, org_magic->max_page_idx ) + 1;
			org_magic->logger_id        = new_magic->logger_id     = 0;
			org_magic->logger_serial    = new_magic->logger_serial = rand();
			org_magic->logger_first_idx = page_index;
			org_magic->logger_magic_idx = save_page( snapshot, snapshot + snapshot_size, page_file, page_index ) - 1;
			if ( org_magic->logger_first_idx <= org_magic->logger_magic_idx )
			{
				page_file->write( 0, org_magic );
				page_file->sync();
			}
			return PREPARED;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	State commit( time_t timestamp )
	{
		try
		{
			page_index_t logger_first_idx = org_magic->logger_first_idx;
			page_index_t logger_magic_idx = org_magic->logger_magic_idx;
			if ( logger_first_idx <= logger_magic_idx )
			{
				restore_page( page_file, logger_first_idx, logger_magic_idx, page_file );
				page_file->read( logger_magic_idx, org_magic );
				org_magic->logger_last_check = timestamp;
				page_file->write( 0, org_magic );
				page_file->truncate( logger_first_idx );
				page_file->sync();
			}
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	State abort()
	{
		try
		{
			page_index_t logger_first_idx = org_magic->logger_first_idx;
			org_magic->logger_first_idx = 0;
			org_magic->logger_magic_idx = 0;
			page_file->write( 0, org_magic );
			page_file->truncate( logger_first_idx );
			page_file->sync();
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}
};

class GlobalLogger : public Logger
{
	class LogInfo
	{
		PageFile	*page_file;
		PageCache	*page_cache;
		void		*key_page;
		page_index_t	key_page_index;
	public:
		~LogInfo() { PageMemory::free( key_page ); }
		LogInfo( PageFile *file, PageCache *cache ) : page_file(file), page_cache(cache), key_page(PageMemory::alloc()) { }

		logger_hdr* logger_item( page_index_t i ) { return (logger_hdr *)( (byte_t *)key_page + *(size32_t *)key_page ) + i; }

		page_index_t prepare( PageFile *log_file, page_index_t page_index, page_index_t rec_index )
		{
			size_t snapshot_size;
			Page** snapshot = page_cache->snapshot_reference(snapshot_size);
			logger_item( rec_index )->logger_first_idx = page_index;
			logger_item( rec_index )->logger_last_idx  = page_index = save_page(snapshot, snapshot + snapshot_size, log_file, page_index);
			log_file->write ( key_page_index, key_page );
			return page_index;
		}

		void commit( PageFile *log_file, time_t _logger_id, time_t timestamp, page_index_t rec_index )
		{
			PageLayout *layout = (PageLayout *)PageMemory::alloc();
			page_index_t it = logger_item( rec_index )->logger_first_idx;
			page_index_t ie = logger_item( rec_index )->logger_last_idx - 1;
			try { page_file->read( 0, layout ); } catch ( PageFile::Exception e ) { }
			layout->logger_id = logger_id() = _logger_id;
			if ( it <= ie )
			{
				page_file->write( 0, layout );
				page_file->sync();
				restore_page( log_file, it, ie, page_file );
				log_file->read( ie, layout );
				layout->logger_id = logger_id() = _logger_id;
			}
			layout->logger_last_check = logger_last_check() = timestamp;
			page_file->write( 0, layout );
			page_file->sync();
			PageMemory::free( layout );
		}

		PageLayout *read_magic( PageFile *log_file, page_index_t rec_index )
		{
			page_index_t it = logger_item( rec_index )->logger_first_idx;
			page_index_t ie = logger_item( rec_index )->logger_last_idx - 1;
			if ( it > ie )
				return NULL;
			PageLayout *layout = (PageLayout *)PageMemory::alloc();
			log_file->read( ie, layout );
			return layout;
		}

		size_t init( PageFile *log_file, page_index_t page_index )
		{
			memset( key_page, 0, PAGESIZE );
			size32_t *lp = (size32_t *)key_page;
			size_t len = strlen( identity() ) + 1;
			*lp = (len + sizeof(size32_t) + 15) & ~15;
			memcpy( lp + 1, identity(), len );
			log_file->write( key_page_index = page_index, key_page );
			return *lp;
		}

		void load( page_index_t page_index, void *page )
		{
			memcpy( key_page, page, PAGESIZE );
			key_page_index = page_index;
		}

		time_t& logger_id()          { return page_cache->magic_layout_ptr()->logger_id; }
		time_t& logger_last_check()  { return page_cache->magic_layout_ptr()->logger_last_check; }
		const char *identity() const { return page_file->identity(); }
		const char *fake_identity() const { return (const char *)key_page + sizeof(size32_t); }

		bool compare_loginfo ( const LogInfo* rhs ) const { return strcmp( identity(), rhs->identity() ) < 0; }
		bool compare_identity( const char*    rhs ) const { return strcmp( identity(), rhs ) < 0; }
	};
	typedef std::vector<LogInfo *> LogInfoVec;
	const char *logdir;
	size_t rotate_pages;
	PageFile *log_file;
	time_t time_stamp;
	PageLayout *key_page;
	bool need_rotate;
	LogInfoVec loginfo_vec;

	page_index_t& logger_page_first() { return key_page->logger_page_first; }
	page_index_t& logger_page_last()  { return key_page->logger_page_last;  }
	page_index_t& logger_rec_max()    { return key_page->logger_rec_max;    }
	page_index_t& logger_rec_cur()    { return key_page->logger_rec_cur;    }
	time_t&       logger_chain()      { return key_page->logger_chain;      }
	logger_hdr*   logger_item( page_index_t i ) { return key_page->logger_head + i; }

	LogInfo *dbfile2loginfo ( const char *dbfile )
	{
		for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
			if ( !strcmp( dbfile, (*it)->fake_identity() ) )
				return *it;
		return NULL;
	}

	bool read_log_file( time_t new_time_stamp )
	{
		open_log_file( new_time_stamp, O_RDONLY );
		log_file->read( 0, key_page );
		for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
			delete *it;
		void *page = PageMemory::alloc();
		for ( page_index_t page_index = 1; page_index < logger_page_first(); page_index++ )
		{
			log_file->read( page_index, page );
			LogInfo *info = new LogInfo( NULL, NULL );
			info->load( page_index, page );
			loginfo_vec.push_back( info );
		}
		PageMemory::free( page );
		return true;
	}

	void open_log_file( time_t new_time_stamp, int flag = O_CREAT|O_RDWR )
	{
		delete log_file;
		char *name = (char *)malloc( strlen(logdir) + 32 );
		sprintf(name, "%slog.%08lx", logdir, new_time_stamp );
		log_file = new PageFile( name, flag );
		free(name);
	}
	
	bool load_log_file( time_t new_time_stamp )
	{
		bool r = true;
		open_log_file( time_stamp = new_time_stamp );
		log_file->read( 0, key_page );
		char *page = (char *)PageMemory::alloc();
		for ( page_index_t page_index = 1; page_index < logger_page_first(); page_index++ )
		{
			log_file->read( page_index, page );
			LogInfoVec::iterator it = std::lower_bound( loginfo_vec.begin(), loginfo_vec.end(), page + sizeof(size32_t), 
					std::mem_fun(&LogInfo::compare_identity) );
			if ( it != loginfo_vec.end() && strcmp( page + sizeof(size32_t), (*it)->identity() ) >= 0 )
				(*it)->load( page_index, page );
			else
				r = false;
		}
		PageMemory::free( page );
		return r;
	}

	void init_log_file( time_t new_time_stamp )
	{
		if ( time_stamp >= new_time_stamp )
			new_time_stamp = time_stamp + 1;
		if ( log_file )
		{
			logger_chain() = new_time_stamp;
			log_file->write ( 0, key_page );
		}
		open_log_file( new_time_stamp );
		memset( key_page, 0, PAGESIZE );
		logger_page_first() = logger_page_last() = loginfo_vec.size() + 1;
		time_stamp = new_time_stamp;
		size_t used_max = 32;
		page_index_t page_index = 1;
		for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
			used_max = std::max( used_max, (*it)->init( log_file, page_index++ ) );
		key_page->logger_rec_max = (PAGESIZE - used_max) / sizeof(logger_hdr);
		log_file->write( 0, key_page );
		log_file->sync();
	}

	State restore( time_t timestamp, page_index_t rec_index )
	{
		try
		{
			time_t logger_id = get_logger_id();
			for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
				(*it)->commit( log_file, logger_id, timestamp, rec_index );
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	page_index_t logger_check_rotate()
	{
		if ( need_rotate || logger_rec_cur() >= logger_rec_max() || logger_page_last() > rotate_pages )
			init_log_file ( time(NULL) );
		return logger_page_last();
	}
public:
	~GlobalLogger()
	{
		for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
			delete *it;
		free((void *)logdir);
		delete log_file;
		PageMemory::free ( key_page );
	}

	GlobalLogger( const char *prefix, size_t rotate = 4096 ) : logdir( strdup(prefix) ), rotate_pages(rotate), 
	       log_file(NULL), time_stamp(time(NULL)), key_page( (PageLayout *)PageMemory::alloc() ), need_rotate(false) { }
	void init( PageFile *file, PageCache *cache ) { loginfo_vec.push_back ( new LogInfo( file, cache ) ); }

	State integrity_verify()
	{
		try
		{
			std::sort( loginfo_vec.begin(), loginfo_vec.end(), std::mem_fun(&LogInfo::compare_loginfo) );
			time_t max_logger_id = 0;
			for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
				max_logger_id = std::max( max_logger_id, (*it)->logger_id() );
			if ( max_logger_id == 0 )
			{
				init_log_file( time(NULL) );
				return CLEAN;
			}
			for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
			{
				time_t cur_logger_id = (*it)->logger_id();
				if ( cur_logger_id == 0 )
					need_rotate = true;
				else if ( cur_logger_id < max_logger_id ) //after logrotate, prepare succeed, but commit fail
					(*it)->logger_id() = max_logger_id;
			}
			if ( !load_log_file(max_logger_id) )
				return CORRUPT;			// some dbfile miss

			page_index_t rec_index = logger_rec_cur();
			switch ( logger_item( rec_index )->check_result )
			{
				case LCR_NULL:
					if ( logger_item( rec_index )->logger_first_idx )
					{
						logger_item( rec_index )->check_result = LCR_ABORT;
						logger_rec_cur() ++;
						log_file->write ( 0, key_page );
						log_file->sync( );
					}
					break;
				case LCR_PREPARED:
					return REDO;
			}
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	State truncate() { return CORRUPT; }

	State prepare()
	{
		try
		{
			page_index_t page_index = logger_check_rotate();
			page_index_t rec_index  = logger_rec_cur();
			logger_item( rec_index )->logger_first_idx = page_index;
			log_file->write( 0, key_page );
			log_file->sync();
			for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
				page_index = (*it)->prepare( log_file, page_index, rec_index );
			logger_item( rec_index )->logger_last_idx  = logger_page_last() = page_index;
			logger_item( rec_index )->check_result     = LCR_PREPARED;
			log_file->write( 0, key_page );
			log_file->sync();
			return PREPARED;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	State redo( time_t timestamp )
	{
		try
		{
			time_stamp = ( timestamp <= time_stamp ) ? ( time_stamp + 1 ) : timestamp;
			page_index_t rec_index = logger_rec_cur();
			time_t logger_id = get_logger_id();
			for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
				if ( (*it)->logger_id() )
					(*it)->commit( log_file, logger_id, time_stamp, rec_index );
			logger_item( rec_index )->check_result = LCR_COMMIT;
			logger_item( rec_index )->logger_check_timestamp = time_stamp;
			logger_rec_cur() ++;
			log_file->write( 0, key_page );
			log_file->sync();
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	State commit( time_t timestamp )
	{
		try
		{
			time_stamp = ( timestamp <= time_stamp ) ? ( time_stamp + 1 ) : timestamp;
			page_index_t rec_index = logger_rec_cur();
			time_t logger_id = get_logger_id();
			for ( LogInfoVec::iterator it = loginfo_vec.begin(), ie = loginfo_vec.end(); it != ie; ++it )
				(*it)->commit( log_file, logger_id, time_stamp, rec_index );
			logger_item( rec_index )->check_result = LCR_COMMIT;
			logger_item( rec_index )->logger_check_timestamp = time_stamp;
			logger_rec_cur() ++;
			log_file->write( 0, key_page );
			log_file->sync();
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	State abort()
       	{
		try
		{
			logger_item( logger_rec_cur()++ )->check_result = LCR_ABORT;
			log_file->write( 0, key_page );
			log_file->sync();
			return CLEAN;
		}
		catch ( PageFile::Exception e ) { }
		return CORRUPT;
	}

	std::vector<time_t> check_version()
	{
		std::vector<time_t> r;
		LogInfoVec::iterator ii  = loginfo_vec.begin();
		time_t logger_last_check = (*ii)->logger_last_check();
		time_t logger_id         = (*ii)->logger_id();
		logger_hdr *it = logger_item(0);
		logger_hdr *ie = logger_item( logger_rec_cur() );
		while ( it != ie && (*it).logger_check_timestamp <= logger_last_check ) ++it;
		do
			if ( (*it).check_result == LCR_COMMIT )
				r.push_back ( (*it).logger_check_timestamp );
		while ( logger_chain() && load_log_file(logger_chain()) && (it = logger_item(0), ie = logger_item(logger_rec_cur()), 1) );
		load_log_file ( logger_id );
		return r;
	}

	std::vector<time_t> restore( time_t timestamp )
	{
		std::vector<time_t> r;
		LogInfoVec::iterator ii  = loginfo_vec.begin();
		time_t logger_last_check = (*ii)->logger_last_check();
		time_t logger_id         = (*ii)->logger_id();
		time_t t = 0;
		bool   c = true;
		logger_hdr *it = logger_item(0);
		logger_hdr *ib = it;
		logger_hdr *ie = logger_item( logger_rec_cur() );
		while ( it != ie && (*it).logger_check_timestamp <= logger_last_check ) ++it;
		do
			for ( ; c && it != ie; ++it )
				if ( (c = ( (*it).logger_check_timestamp < timestamp) ) && (*it).check_result == LCR_COMMIT &&
						(c = ( restore( t = (*it).logger_check_timestamp, it - ib ) == Logger::CLEAN )))
					r.push_back( t );
		while ( c && logger_chain() && load_log_file(logger_chain()) && (it = ib = logger_item(0), ie = logger_item(logger_rec_cur()), 1) );
		load_log_file ( logger_id );
		return r;
	}

	std::vector<PageLayout *> read_magic( time_t timestamp, time_t start, const char *dbfile )
	{
		std::vector<PageLayout *> r;
		PageLayout *layout;
		read_log_file( timestamp );
		logger_hdr *it = logger_item(0);
		logger_hdr *ib = it;
		logger_hdr *ie = logger_item( logger_rec_cur() );
		while ( it != ie && (*it).logger_check_timestamp <= start ) ++it;
		do
			if ( LogInfo *loginfo = dbfile2loginfo( dbfile ) )
				for ( ; it != ie; ++it )
					if ( (*it).check_result == LCR_COMMIT && (layout = loginfo->read_magic( log_file, it - ib ) ) )
						r.push_back ( layout );
		while ( logger_chain() && read_log_file(logger_chain()) && (it = ib = logger_item(0), ie = logger_item( logger_rec_cur()), 1) );
		return r;
	}

	time_t get_time_stamp() const { return time_stamp; }
	time_t get_logger_id() const
	{
		time_t ts;
		sscanf( log_file->identity(), "log.%lx", &ts );
		return ts;
	}
};

class PerformanceMonitor
{
	PageLayout *org_layout;
	PageLayout *cur_layout;
	Performance perf_org;
	Performance perf_cur;
	time_t      time_org;
	time_t      time_cur;
	time_t      time_new;
	size_t      perf_org_elapse;
	size_t      perf_cur_elapse;
public:
	~PerformanceMonitor () { PageMemory::free(org_layout); PageMemory::free(cur_layout); }
	PerformanceMonitor() : org_layout( (PageLayout *)PageMemory::alloc() ), cur_layout( (PageLayout *)PageMemory::alloc() ),
		time_org(0), time_cur(0), time_new(0) { }

	void init( PageLayout *new_layout )
	{
		memcpy( org_layout, new_layout, PAGESIZE );
		memcpy( cur_layout, new_layout, PAGESIZE );
		time_org = org_layout->logger_last_check;
	}

	bool update( PageLayout *new_layout )
	{
		if ( new_layout->logger_last_check <= cur_layout->logger_last_check )
			return false;
		perf_org  = new_layout->performance;
		perf_cur  = new_layout->performance;
		perf_org -= org_layout->performance;
		perf_cur -= cur_layout->performance;
		time_cur  = cur_layout->logger_last_check;
		time_new  = new_layout->logger_last_check;
		memcpy ( cur_layout, new_layout, PAGESIZE );
		return true;
	}

	const Performance *performance_from_begin( time_t *t_org, time_t *t_new ) const { *t_org = time_org; *t_new = time_new; return &perf_org; }
	const Performance *performance_from_checkpoint( time_t *t_cur, time_t *t_new ) const { *t_cur = time_cur; *t_new = time_new; return &perf_cur; }
};

class PageMonitor
{
	PageFile *page_file;
        PageLayout *layout;
	PerformanceMonitor *performance_monitor;
public:
	~PageMonitor () { delete page_file; PageMemory::free(layout); }
	PageMonitor( PerformanceMonitor *monitor, const char *dbfile ) : page_file(new PageFile( dbfile, O_RDONLY )), 
		layout( (PageLayout *)PageMemory::alloc() ), performance_monitor(monitor)
	{
		page_file->read( 0, layout );
		performance_monitor->init( layout );
	}

	bool monitor( )
	{
		page_file->read( 0, layout );
		return performance_monitor->update( layout );
	}
};

class LoggerMonitor
{
	std::vector<PageLayout *> magics;
	std::vector<PageLayout *>::iterator iterator;
	PerformanceMonitor *performance_monitor;
public:
	~LoggerMonitor()
	{
		for ( std::vector<PageLayout *>::iterator it = magics.begin(), ie = magics.end(); it != ie; ++it )
			delete *it;
	}
	LoggerMonitor( PerformanceMonitor *monitor, const char *dbfile, const char *logdir, time_t t_start = 0 ) : performance_monitor(monitor)
	{
		std::vector<time_t> r;
		if ( DIR *dir = opendir( logdir ) )
		{
			while ( dirent *d = readdir( dir ) )
				if ( d->d_name[0] != '.' )
				{
					time_t ts;
					(void)sscanf( d->d_name, "log.%lx", &ts );
					r.push_back( ts );
				}
			closedir( dir );
		}
		std::sort( r.begin(), r.end() );
		std::vector<time_t>::iterator it = std::lower_bound( r.begin(), r.end(), t_start );
		if ( it != r.end() )
		{
			if ( it != r.begin() )
				--it;
			magics = GlobalLogger( logdir ).read_magic( *it, t_start, dbfile );
		}
		iterator = magics.begin();
		if ( iterator != magics.end() && (*iterator)->logger_last_check == 0 ) ++iterator;
		if ( iterator != magics.end() ) performance_monitor->init( *iterator++ );
	}

	bool monitor( )
	{
		if ( iterator == magics.end() )
			return false;
		while ( !performance_monitor->update( *iterator++ ) && iterator != magics.end() );
		return true;
	}
};

class PageBrowser
{
	PageFile	*file;
	size_t		cache_max;	
	PageHash	hash;

	bool is_data_page( data_hdr *hdr )
	{
		data_hdr *cur = hdr;
		for ( int i = 0; i < PAGEUSED / 16; i++ )
		{
			cur = cur->next_head();
			if ( !cur ) return true;
			ptrdiff_t offset = (byte_t *)cur - (byte_t *)hdr;
			if ( offset > PAGEUSED || offset < 0 ) return false;
		}
		return false;
	}

	data_hdr* load( page_index_t page_index )
	{
		Page *page = hash.find( page_index );
		if ( !page )
		{
			page = new Page( page_index, NULL );
			try { file->read( page_index, page->layout_ptr() ); }
			catch ( PageFile::Exception e ) { delete page; throw; }
			if ( !is_data_page( page->layout_ptr()->data ) )
			{
				delete page;
				return NULL;
			}
			hash.insert( page );
		}
		return page->layout_ptr()->data;
	}

	data_hdr* next_data_head( data_hdr *hdr )
	{
		if ( hdr->next_page_index )
			try
			{
				if ( data_hdr *next = load( hdr->next_page_index ) )
					return ( data_hdr * )((byte_t *)next + hdr->next_page_pos );
			}
			catch ( PageFile::Exception e ) { }
		return NULL;
	}

	void* extract_data( data_hdr *hdr )
	{
		size_t size = *(size32_t *)(hdr + 1);
		byte_t *data = (byte_t *)malloc( size );
		for ( byte_t *p = data; hdr; p += hdr->size, hdr = next_data_head( hdr ) )
		{
			if ( size < hdr->size )
			{
				free(data);
				return NULL;
			}
			memcpy( p, hdr + 1, hdr->size );
			size -= hdr->size;
		}
		if ( size )
		{
			free(data);
			return NULL;
		}
		return data;
	}
public:
	~PageBrowser() { delete file; }
	PageBrowser( const char *filename, size_t max = 1024 ) : file(new PageFile( filename, O_RDONLY ) ), cache_max(max)
	{
		hash.init( max, max );
	}

	size_t action( IQueryData *query )
	{
		int corrupt_count = 0;
		try
		{
			for ( page_index_t page_index = 1; ; page_index ++ )
			{
				if ( data_hdr *hdr = load(page_index) )
				{
					do
						if ( hdr->size && hdr->first_slice )
						{
							void	*data = extract_data( hdr );
							if ( data )
							{
								size_t	key_len = *((size32_t *)data + 1);
								size_t	val_len = *(size32_t *)data - key_len - sizeof(size32_t) - sizeof(size32_t);
								void	*key = (byte_t *)data + sizeof(size32_t) + sizeof(size32_t);
								void	*val = (byte_t *)key + key_len;
								query->update( key, key_len, val, val_len );
								free(data);
							}
							else
								corrupt_count++;
						}
					while ( (hdr = hdr->next_head()) );
				}
				if ( hash.size() > cache_max )
					hash.clear();
			}
		}
		catch (...) { }
		return corrupt_count;
	}
};

class PageRebuild
{
	class PageWriter : public IQueryData
	{
		PageFile  *file;
		PageCache *cache;
		Logger    *logger;
		size_t	  count;
		size_t    cache_max;
	public:
		~PageWriter()
		{
			cache->snapshot_create(); logger->prepare(); logger->commit(time(NULL)); cache->snapshot_release();
			delete cache;
			delete file;
			delete logger;
		}

		PageWriter( const char *dst, size_t max ) : 
			file(new PageFile(dst)), cache( new PageCache( file, max, max / 2 ) ), logger(new NullLogger()), count(0), cache_max(max)
		{
			logger->init( file, cache );
		}

		bool update( const void *key, size_t key_len, const void *val, size_t val_len )
		{
			cache->put( key, key_len, val, val_len );
			if ( ++count % cache_max == 0 )
			{
				cache->snapshot_create(); logger->prepare(); logger->commit(time(NULL)); cache->snapshot_release();
			}
			return true;
		}

		size_t counter() const { return count; }
	};

	PageBrowser *browser;
	PageWriter  *writer;
public:
	~PageRebuild()
	{
		delete browser;
		delete writer;
	}
	PageRebuild( const char *dst, const char *src, size_t max = 1024 ) : browser(new PageBrowser(src, max)), writer(new PageWriter(dst, max)) { }

	size_t action(size_t *corrupt_count)
	{
		size_t c = browser->action( writer );
		if ( corrupt_count ) *corrupt_count = c;
		return writer->counter();
	}
};

inline data_hdr* data_hdr::next_head() { return next != PAGEMAXSPARE + sizeof(data_hdr) ? (data_hdr *)((byte_t *)layout_ptr(this) + next) : NULL; }
inline size_t data_hdr::capacity() { return next - layout_offset(this) - sizeof(data_hdr); }
inline data_hdr* data_hdr::Rnext_page_head() { return (data_hdr *)Layout2PageCache(this)->Rload_hdr(next_page_index, next_page_pos); }
inline data_hdr* data_hdr::Wnext_page_head() { return (data_hdr *)Layout2PageCache(this)->Wload_hdr(next_page_index, next_page_pos); }
inline frag_hdr* frag_hdr::Wnext_page_head() { return (frag_hdr *)Layout2PageCache(this)->Wload_hdr(next_page_index, next_page_pos); }
inline frag_hdr* frag_hdr::Wprev_page_head() { return (frag_hdr *)Layout2PageCache(this)->Wload_hdr(prev_page_index, prev_page_pos); }
inline data_hdr* index_hdr::Rindex_hdr2data_hdr() { return (data_hdr *)Layout2PageCache(this)->Rload_hdr(page_index, page_pos); }
inline data_hdr* index_hdr::Windex_hdr2data_hdr() { return (data_hdr *)Layout2PageCache(this)->Wload_hdr(page_index, page_pos); }

inline Page*        Layout2Page     ( void *p ) { return *(Page **)( (PageLayout *)((ptrdiff_t)p & PAGEMASK) + 1 ); }
inline PageCache*   Layout2PageCache( void *p ) { return Layout2Page(p)->cache(); }
inline page_index_t Layout2PageIndex( void *p ) { return Layout2Page(p)->index(); }
inline void* layout_ptr( void *p ) { return (void *)((ptrdiff_t)p & PAGEMASK); }
inline page_pos_t layout_offset( void * p ) { return (ptrdiff_t)p & ~PAGEMASK; }

inline void free_data ( data_hdr *hdr )
{
	PageCache *cache = Layout2PageCache(hdr);
	do
	{
		hdr->first_slice             = 0;
		hdr->size                    = 0;
		data_hdr *next, *cur         = (data_hdr *)layout_ptr(hdr);
		if ( hdr != cur )
		{
			for ( next = cur->next_head(); next != hdr; next = next->next_head() ) cur = next;
			if (  cur->size == 0 )
			{
				cache->clr_fragment((frag_hdr *)cur);
				cur->next = next->next;
			}
			else
				cur = next;
		}
		if ( (next = cur->next_head()) && next->size == 0 )
		{
			cache->clr_fragment((frag_hdr *)next);
			cur->next = next->next;
		}
		hdr = hdr->next_page_index ? hdr->Wnext_page_head() : NULL;
		cache->set_fragment((frag_hdr *)cur);
	} while ( hdr );
}

inline void* extract_data( data_hdr *hdr, size_t &size, void *data_buf )
{
	size_t copy_len = *(size32_t *)(hdr + 1);
	byte_t *data = (byte_t *)(copy_len <= size ? data_buf : malloc( copy_len ));
	size = copy_len;
	for ( byte_t *p = data; ; )
	{
		memcpy( p, hdr + 1, hdr->size );
		p += hdr->size;
		if ( ! hdr->next_page_index )
			break;
		hdr = hdr->Rnext_page_head();
	}
	return data;
}

inline void* extract_key( data_hdr *hdr, size_t &key_len )
{
	size_t copy_len = key_len = *((size32_t *)(hdr + 1) + 1);
	byte_t *key = (byte_t *)malloc( key_len );
	byte_t *p   = key;
	size_t min  = hdr->size - sizeof(size32_t) - sizeof(size32_t);
	if ( copy_len <= min )
		memcpy( p, (size32_t *)(hdr + 1) + 2, copy_len );
	else
	{
		memcpy( p, (size32_t *)(hdr + 1) + 2, min );
		p        += min;
		copy_len -= min;
		while ( copy_len )
		{
			hdr       = hdr->Rnext_page_head();
			min       = copy_len < hdr->size ? copy_len : hdr->size;
			memcpy( p, hdr + 1, min );
			p        += min;
			copy_len -= min;
		}
	}
	return key;
}

inline void* extract_key( data_hdr *hdr, size_t &key_len, void *key_buf )
{
	size_t copy_len = *((size32_t *)(hdr + 1) + 1);
	byte_t *key = (byte_t *)(copy_len <= key_len ? key_buf : malloc( copy_len ));
	key_len     = copy_len;
	byte_t *p   = key;
	size_t min  = hdr->size - sizeof(size32_t) - sizeof(size32_t);
	if ( copy_len <= min )
		memcpy( p, (size32_t *)(hdr + 1) + 2, copy_len );
	else
	{
		memcpy( p, (size32_t *)(hdr + 1) + 2, min );
		p        += min;
		copy_len -= min;
		while ( copy_len )
		{
			hdr       = hdr->Rnext_page_head();
			min       = copy_len < hdr->size ? copy_len : hdr->size;
			memcpy( p, hdr + 1, min );
			p        += min;
			copy_len -= min;
		}
	}
	return key;
}

inline void* extract_val( data_hdr *hdr, size_t &val_len )
{
	size_t size     = *(size32_t *)(hdr + 1);
	size_t key_len  = *((size32_t *)(hdr + 1) + 1);
	byte_t *val     = (byte_t *)malloc(val_len = size - key_len - sizeof(size32_t) - sizeof(size32_t));
	byte_t *p       = val;
	size_t pass_len = key_len + sizeof(size32_t) + sizeof(size32_t);
	for ( ;pass_len > hdr->size; hdr = hdr->Rnext_page_head() ) pass_len -= hdr->size;
	memcpy ( p, (byte_t *)(hdr + 1) + pass_len, hdr->size - pass_len );
	for ( p += hdr->size - pass_len; hdr->next_page_index; p += hdr->size )
	{
		hdr = hdr->Rnext_page_head();
		memcpy( p, hdr + 1, hdr->size );
	}
	return val;
}

inline void* extract_val( data_hdr *hdr, size_t &val_len, void *val_buf )
{
	size_t size     = *(size32_t *)(hdr + 1);
	size_t key_len  = *((size32_t *)(hdr + 1) + 1);
	size_t copy_len = size - key_len - sizeof(size32_t) - sizeof(size32_t);
	byte_t *val     = (byte_t *)(copy_len <= val_len ? val_buf :  malloc(copy_len));
	val_len         = copy_len;
	byte_t *p       = val;
	size_t pass_len = key_len + sizeof(size32_t) + sizeof(size32_t);
	for ( ;pass_len > hdr->size; hdr = hdr->Rnext_page_head() ) pass_len -= hdr->size;
	memcpy ( p, (byte_t *)(hdr + 1) + pass_len, hdr->size - pass_len );
	for ( p += hdr->size - pass_len; hdr->next_page_index; p += hdr->size )
	{
		hdr = hdr->Rnext_page_head();
		memcpy( p, hdr + 1, hdr->size );
	}
	return val;
}

inline void* extract_data( index_hdr *hdr, size_t &size, void *data_buf   ) { return extract_data( hdr->Rindex_hdr2data_hdr(), size, data_buf   ); }
inline void* extract_key ( index_hdr *hdr, size_t &key_len ) { return extract_key ( hdr->Rindex_hdr2data_hdr(), key_len ); }
inline void* extract_key ( index_hdr *hdr, size_t &key_len, void *key_buf ) { return extract_key ( hdr->Rindex_hdr2data_hdr(), key_len, key_buf ); }
inline void* extract_val ( index_hdr *hdr, size_t &val_len ) { return extract_val ( hdr->Rindex_hdr2data_hdr(), val_len ); }
inline void* extract_val ( index_hdr *hdr, size_t &val_len, void *val_buf ) { return extract_val ( hdr->Rindex_hdr2data_hdr(), val_len, val_buf ); }

inline Performance* Page::performance() { return page_cache->performance(); }
inline Page* Page::RL_page( index_hdr *hdr ) { return page_cache->Rload( L_index(hdr) ); }
inline Page* Page::RR_page( index_hdr *hdr ) { return page_cache->Rload( R_index(hdr) ); }
inline Page* Page::RI_page( page_index_t page_index ) { return page_cache->Rload( page_index ); }
inline Page* Page::WL_page( index_hdr *hdr ) { return page_cache->Wload( L_index(hdr) ); }
inline Page* Page::WR_page( index_hdr *hdr ) { return page_cache->Wload( R_index(hdr) ); }
inline Page* Page::WI_page( page_index_t page_index ) { return page_cache->Wload( page_index ); }
inline Page* Page::alloc_root_index_page() { return page_cache->alloc_root_index_page(); }
inline Page* Page::alloc_page() { return page_cache->alloc(); }
inline void Page::free_page( Page *page ) { page_cache->free(page); }
inline void Page::set_root_index( Page *page ) { page_cache->set_root_index(page); }

inline int memcmp_fast( const void *s1, const void *s2, size_t n )
{
	while ( n >= sizeof(dword_t) )
	{
		dword_t k1 = *(dword_t *)s1;
		dword_t k2 = *(dword_t *)s2;
		if ( dword_t comp = k1 - k2 )
		{
			if ( comp & 0xFF     ) return (k1 & 0xFF    ) - (k2 &0xFF    );
			if ( comp & 0xFF00   ) return (k1 & 0xFF00  ) - (k2 &0xFF00  );
			if ( comp & 0xFF0000 ) return (k1 & 0xFF0000) - (k2 &0xFF0000);
			return (k1>>24) - (k2>>24);
		}
		s1 = (dword_t *)s1 + 1;
		s2 = (dword_t *)s2 + 1;
		n -= sizeof(dword_t);
	}
	while ( n )
	{
		if ( int comp = *(byte_t *)s1 - *(byte_t *)s2 )
			return comp;
		s1 = (byte_t *)s1 + 1;
		s2 = (byte_t *)s2 + 1;
		n -= sizeof(byte_t);
	}
	return 0;
}

inline int compare( index_hdr *hdr, const void *key, size_t key_len )
{
	if ( size_t i_key_len = hdr->key_len )
	{
		if ( int comp = memcmp_fast( key, hdr->key, std::min(key_len, i_key_len) ) )
			return comp;
		return key_len - i_key_len;
	}
	if ( key_len <= sizeof(hdr->key) )
	{
		if ( int comp = memcmp_fast( key, hdr->key, key_len ) )
			return comp;
		return -1;
	}
	if ( int comp = memcmp_fast( key, hdr->key, sizeof(hdr->key)) )
		return comp;
	data_hdr *ddr    = hdr->Rindex_hdr2data_hdr();
	size_t i_key_len = *((size32_t *)(ddr + 1) + 1);
	size_t cmp_len   = std::min( key_len, i_key_len );
	size_t min       = ddr->size - sizeof(size32_t) - sizeof(size32_t);
	byte_t *i_key    = (byte_t *)(ddr + 1) + sizeof(size32_t) + sizeof(size32_t);
	if ( cmp_len <= min )
	{
		if ( int comp = memcmp_fast( key, i_key, cmp_len ) )
			return comp;
		return key_len - i_key_len;
	}
	if ( int comp = memcmp_fast( key, i_key, min ) )
		return comp;
	while ( cmp_len -= min )
	{
		key = (byte_t *)key + min;
		ddr = ddr->Rnext_page_head();
		min = cmp_len < ddr->size ? cmp_len : ddr->size;
		if ( int comp = memcmp_fast( key, ddr + 1, min ) )
			return comp;
	}
	return key_len - i_key_len;
}

}; // end of __db_helper

using __db_helper::page_index_t;
using __db_helper::PageCache;
using __db_helper::PageFile;
using __db_helper::Logger;
using __db_helper::PageLogger;
using __db_helper::GlobalLogger;
using __db_helper::PageBrowser;
using __db_helper::PageRebuild;
using __db_helper::Performance;
using __db_helper::PerformanceMonitor;
using __db_helper::PageMonitor;
using __db_helper::LoggerMonitor;
using __db_helper::IQueryKey;
using __db_helper::IQueryData;

class DB
{
public:
	PageCache *page_cache;
	virtual ~DB() { }
	virtual bool init() { return true; }
	bool put( const void* key, size_t key_len, const void* val, size_t val_len, bool replace = true )
	{
		return page_cache->put( key, key_len, val, val_len, replace );
	}
	void* put( const void *key, size_t key_len, const void* val, size_t *val_len )
	{
		return page_cache->put( key, key_len, val, *val_len );
	}
	void* find( const void* key, size_t key_len, size_t *val_len )
	{
		return page_cache->find( key, key_len, *val_len );
	}
	void* find( const void* key, size_t key_len, size_t *val_len, void *val_buf )
	{
		return page_cache->find( key, key_len, *val_len, val_buf );
	}
	void* first_key( size_t *key_len )
	{
		return page_cache->first_key( *key_len );
	}
	void* next_key( const void* key, size_t *key_len )
	{
		return page_cache->next_key( key, *key_len );
	}
	bool  del( const void* key, size_t key_len )  { return page_cache->del( key, key_len ); }
	void* del( const void* key, size_t key_len, size_t *val_len ) { return page_cache->del( key, key_len, *val_len ); }
	bool exist( const void* key, size_t key_len ) { return page_cache->exist( key, key_len ); }
	void walk ( IQueryKey  *query ) { page_cache->walk( query ); }
	void walk ( IQueryData *query ) { page_cache->walk( query ); }
	void walk ( const void *key, size_t key_len, IQueryKey  *query ) { page_cache->walk( key, key_len, query ); }
	void walk ( const void *key, size_t key_len, IQueryData *query ) { page_cache->walk( key, key_len, query ); }
	size_t record_count() const { return page_cache->record_count(); }
	void snapshot_create()  { page_cache->snapshot_create(); }
	void snapshot_release() { page_cache->snapshot_release(); }
	void performance_dump() { return page_cache->performance()->dump(); }
};

class DBStandalone : public DB
{
	size_t cache_high, cache_low;
	PageFile  *page_file;
	Logger    *logger;
public:
	~DBStandalone()
	{
		delete page_cache;
		delete page_file;
		delete logger;
	}

	DBStandalone( const char *dbfile, size_t high = 1024, size_t low = 768 ) : cache_high( high ), cache_low( low ),
		page_file( new PageFile(dbfile) ), logger( new PageLogger() )
	{
		page_cache = new PageCache( page_file, cache_high, cache_low );
		logger->init( page_file, page_cache );
	}

	bool init()
	{
		Logger::State state = logger->integrity_verify();
		if ( state != PageLogger::CLEAN )
		{
			if ( state == PageLogger::CORRUPT )
				state = logger->truncate();
			else if ( state == PageLogger::REDO )
				state = logger->redo( time(NULL) );
			delete page_cache;
			if ( state == PageLogger::CLEAN )
				page_cache = new PageCache(page_file, cache_high, cache_low);
		}
		return state == Logger::CLEAN;
	}

	bool checkpoint_prepare() { return logger->prepare()          == Logger::PREPARED; }
	bool checkpoint_commit()  { return logger->commit(time(NULL)) == Logger::CLEAN; }
	bool checkpoint()
	{
		snapshot_create();
		if ( !checkpoint_prepare() )
		{
			snapshot_release();
			return false;
		}
		bool r = checkpoint_commit();
		snapshot_release();
		return r;
	}
};

class DBCollection
{
	struct Table
	{
		const char *table_name;
		size_t cache_high, cache_low;
		PageFile  *page_file;
		PageCache *page_cache;
		DB	  *db;
		~Table() { close(); free((void *)table_name); delete db; }
		Table( const char *name, size_t high, size_t low ) : table_name(strdup(name)), cache_high(high), cache_low(low),
	       		page_file(NULL), page_cache(NULL), db(new DB) { }

		void close()
		{
			delete page_cache;
			delete page_file;
		}

		void open( const char *path, Logger *logger )
		{
			char *buffer = (char *)malloc( strlen(path) + strlen(table_name) + 1 );
			strcpy( buffer, path );
			strcat( buffer, table_name );
			page_file      = new PageFile( buffer );
			page_cache     = new PageCache( page_file, cache_high, cache_low );
			db->page_cache = page_cache;
			logger->init( page_file, page_cache );
			free(buffer);
		}

		bool compare_table( const Table* rhs ) const { return strcmp( table_name, rhs->table_name ) < 0; }
		bool compare_name ( const char*  rhs ) const { return strcmp( table_name, rhs ) < 0; }
		void snapshot_create()  { page_cache->snapshot_create(); }
		void snapshot_release() { page_cache->snapshot_release(); }
	};
	typedef std::vector< Table* > TableVec;
	char *datadir, *logdir;
	size_t logpages;
	TableVec table_vec;
	GlobalLogger *logger;

	void unload()
	{
		std::for_each( table_vec.begin(), table_vec.end(), std::mem_fun(&Table::close) );
		delete logger;
	}

	void load()
	{
		create_dir(datadir);
		create_dir(logdir);
		std::sort( table_vec.begin(), table_vec.end(), std::mem_fun(&Table::compare_table) );
		logger = new GlobalLogger( logdir, logpages );
		for ( TableVec::iterator it = table_vec.begin(), ie = table_vec.end(); it != ie; ++it )
			(*it)->open( datadir, logger );
	}
public:
	~DBCollection()
	{
		free(datadir);
		free(logdir);
		for ( TableVec::iterator it = table_vec.begin(), ie = table_vec.end(); it != ie; ++it )
			delete *it;
		delete logger;
	}
	DBCollection(size_t pages = 4096) : datadir(NULL), logdir(NULL), logpages(pages), logger(NULL) { }

	static char *fixup_dir( const char *dir )
	{
		size_t len = strlen(dir);
		if ( dir[ len - 1 ] == '/' )
			return strdup(dir);
		char *p = (char *)malloc( len + 2 );
		memcpy( p, dir, len );
		p[len    ] = '/';
		p[len + 1] = '\0';
		return p;
	}

	static void create_dir( const char *dir )
	{
		char *p = strdup(dir);
		char *q = strdup(dir);
		char *r = p;
		q[ *q == '/' ? 1 : 0 ] = '\0';
		for ( p = strtok(p, "/"); p; p = strtok(NULL, "/") )
		{
			strcat( q, p   );
			strcat( q, "/" );
			mkdir( q, 0755 );
		}
		free(q);
		free(r);
	}

	void set_data_dir ( const char *dir  ) { if ( !datadir ) datadir = fixup_dir(dir); }
	void set_log_dir  ( const char *dir  ) { if ( !logdir  ) logdir  = fixup_dir(dir); }
	void set_table ( const char *name, size_t cache_high, size_t cache_low )
	{
		table_vec.push_back ( new Table( name, cache_high, cache_low ) );
	}

	size_t load_tables()
	{
		if ( DIR *dir = opendir( datadir ) )
		{
			while ( dirent *d = readdir( dir ) )
				if ( d->d_name[0] != '.' )
					table_vec.push_back ( new Table( d->d_name, 0, 0 ) );
			closedir( dir );
		}
		return table_vec.size();
	}

	bool init ()
	{
		load();
		Logger::State state = logger->integrity_verify();
		if ( state != PageLogger::CLEAN )
		{
			if ( state == PageLogger::CORRUPT )
				state = logger->truncate();
			else if ( state == PageLogger::REDO )
				state = logger->redo( time(NULL) );
			if ( state == PageLogger::CLEAN )
			{
				unload();
				load();
			}
		}
		return state == Logger::CLEAN;
	}

	bool checkpoint_prepare()          { return logger->prepare()          == Logger::PREPARED; }
	bool checkpoint_commit()           { return logger->commit(time(NULL)) == Logger::CLEAN; }
	bool checkpoint()
	{
		std::for_each( table_vec.begin(), table_vec.end(), std::mem_fun(&Table::snapshot_create) );
		if ( !checkpoint_prepare() )
		{
			std::for_each( table_vec.begin(), table_vec.end(), std::mem_fun(&Table::snapshot_release) );
			return false;
		}
		bool r = checkpoint_commit();
		std::for_each( table_vec.begin(), table_vec.end(), std::mem_fun(&Table::snapshot_release) );
		return r;
	}

	std::vector<time_t> version () { return logger->check_version(); }
	std::vector<time_t> restore (time_t timestamp) { return logger->restore( timestamp ); }
	std::vector<time_t> remove_logs()
	{
		std::vector<time_t> r;
		time_t time_stamp = logger->get_logger_id();
		if ( DIR *dir = opendir( logdir ) )
		{
			while ( dirent *d = readdir( dir ) )
				if ( d->d_name[0] != '.' )
				{
					time_t ts;
					sscanf( d->d_name, "log.%lx", &ts );
					if ( ts < time_stamp ) r.push_back(ts);
				}
			closedir( dir );
		}
		return r;
	}

	DB *db ( const char *name )
	{
		TableVec::iterator it = std::lower_bound( table_vec.begin(), table_vec.end(), name, std::mem_fun(&Table::compare_name) );
		if ( it != table_vec.end() && strcmp( name, (*it)->table_name ) >= 0 )
			return (*it)->db;
		return NULL;
	}
};

};

#endif

