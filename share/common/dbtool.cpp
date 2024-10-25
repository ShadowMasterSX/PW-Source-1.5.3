#include "db.h"
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace GNET;


void rebuild_usage()
{
	puts("Usage: dbtool <-r|--rebuild> <-s|--srouce> <sourcefile> <-d|--destination> <destinationfile>");
	exit(0);
}

int rebuild_main( int argc, char *argv[] )
{
	option options[] = 
	{ 
		{ "help",        required_argument, NULL, 'h' },
		{ "source",      required_argument, NULL, 's' },
		{ "destination", required_argument, NULL, 'd' },
		NULL
	};
	char *src = NULL, *dst = NULL;
	for ( int n = 0, c; (c = getopt_long( argc, argv, "hs:d:", options, &n )) != -1; )
	{
		switch ( c )
		{
		case 's' : src = strdup(optarg); break;
		case 'd' : dst = strdup(optarg); break;
		default  : rebuild_usage();
		}
	}
	if ( !src || !dst ) rebuild_usage();
	PageRebuild rebuild( dst, src );
	size_t corrupt_count;
	printf("Rebuild %d Items", rebuild.action(&corrupt_count));
	if ( corrupt_count )
		printf(" with %d Corrupt Items\n", corrupt_count );
	else
		printf("\n");
	return 0;
}

void collection_usage()
{
	puts("Usage: dbtool <-c|--collection> <-d|--datadir> <datadir> <-l|--logdir> <logdir> ([-v|--version] | [-r|--restore] <timestamp> | [-g|remove-logs])");
	exit(0);
}

int collection_main( int argc, char *argv[], const char *datadir, const char *logdir )
{
	printf("collection main\n");
	option options[] = 
	{ 
		{ "help",        required_argument, NULL, 'h' },
		{ "version",     no_argument,       NULL, 'v' },
		{ "restore",     required_argument, NULL, 'r' },
		{ "remove-logs", no_argument,       NULL, 'g' },
		NULL
	};

	bool version     = false;
	bool remove_logs = false;
	char *timestamp  = NULL;
	for ( int n = 0, c; (c = getopt_long( argc, argv, "hgvr:", options, &n )) != -1; )
	{
		switch ( c )
		{
		case 'v' : version = true; break;
		case 'r' : timestamp = strdup(optarg); break;
		case 'g' : remove_logs = true; break;
		default  : collection_usage();
		}
	}

	DBCollection *env = new DBCollection();
	env->set_data_dir( datadir );
	env->set_log_dir ( logdir );
	size_t load_tables = env->load_tables();
	bool   init        = env->init();
	printf("load tables = %d\n", load_tables );
	printf("init        = %s\n", init ? "OK" : "ERR" );
	if ( init )
	{
		if ( version )
		{
			std::vector<time_t> r = env->version();
			puts("Version:");
			for ( std::vector<time_t>::iterator it = r.begin(), ie = r.end(); it != ie; ++it )
				printf("\t<%lx> %s", *it, ctime(&*it) );
		}
		else if ( timestamp )
		{
			std::vector<time_t> r = env->restore( (size_t)atoi(timestamp) );
			puts("Restore:");
			for ( std::vector<time_t>::iterator it = r.begin(), ie = r.end(); it != ie; ++it )
				printf("\t<%lx> %s", *it, ctime(&*it) );
		}
		else if ( remove_logs )
		{
			std::vector<time_t> r = env->remove_logs();
			puts("RemoveLogs:");
			char *buffer = (char *)malloc( strlen(logdir) + 32 );
			for ( std::vector<time_t>::iterator it = r.begin(), ie = r.end(); it != ie; ++it )
			{
				sprintf( buffer, "%s/log.%lx", logdir, *it );
				remove( buffer );
				printf("\t<%lx> %s", *it, ctime(&*it) );
			}
			free(buffer);
		}
	}
	delete env;
	return 0;
}

void dbcollection_usage()
{
	puts("Usage: dbtool <-c|--collection> <-d|--datadir> <datadir> <-l|--logdir> <logdir> ...");
	exit(0);
}

int dbcollection_main( int argc, char *argv[] )
{
	option options[] = 
	{ 
		{ "help",        required_argument, NULL, 'h' },
		{ "datadir",     required_argument, NULL, 'd' },
		{ "logdir",      required_argument, NULL, 'l' },
		NULL
	};
	char *datadir = NULL, *logdir = NULL;
	for ( int n = 0, c; (c = getopt_long( argc, argv, "hd:l:", options, &n )) != -1; )
	{
		switch ( c )
		{
		case 'd' : datadir = strdup(optarg); if ( logdir )  goto finish; else break;
		case 'l' : logdir  = strdup(optarg); if ( datadir ) goto finish; else break;
		}
	}
	dbcollection_usage();
finish:
	return collection_main( argc, argv, datadir, logdir );
}

void monitor_usage()
{
	puts("Usage: dbtool <-m|--monitor> <-d|--dbfile> <dbfilepath> [ <-d|--dbfile> <dbfile> <-l|--logdir> <logdir> ]");
	exit(0);
}

int monitor_main( int argc, char *argv[] )
{
	option options[] = 
	{ 
		{ "help",        required_argument, NULL, 'h' },
		{ "dbfile",      required_argument, NULL, 'd' },
		{ "logdir",      required_argument, NULL, 'l' },
		NULL
	};
	char *dbfile = NULL, *logdir = NULL;
	for ( int n = 0, c; (c = getopt_long( argc, argv, "hd:l:", options, &n )) != -1; )
	{
		switch ( c )
		{
		case 'd' : dbfile = strdup(optarg); if ( logdir ) goto finish; else break;
		case 'l' : logdir = strdup(optarg); if ( dbfile ) goto finish; else break;
		}
	}
	if ( !dbfile )
		monitor_usage();
finish:
	PerformanceMonitor pm;
	if ( logdir )
	{
		logdir = DBCollection::fixup_dir(logdir);
		LoggerMonitor monitor( &pm, dbfile, logdir );
		while ( monitor.monitor() )
		{
			time_t t_org, t_cur, t_new;
			const Performance *p1 = pm.performance_from_begin( &t_org, &t_new );
			const Performance *p2 = pm.performance_from_checkpoint( &t_cur, &t_new );
			fprintf(stdout, "Performance from begin ( %ld - %ld ) elapsed %ld second\n", t_org, t_new, t_new - t_org );
			p1->dump( stdout, (double)t_new - t_org );
			fprintf(stderr, "Performance from checkpoint ( %ld - %ld ) elapsed %ld second\n", t_cur, t_new, t_new - t_cur );
			p2->dump( stderr, (double)t_new - t_cur );
		}
	}
	else
	{
		PageMonitor monitor( &pm, dbfile );
		while ( 1 )
		{
			if ( monitor.monitor() )
			{
				time_t t_org, t_cur, t_new;
				const Performance *p1 = pm.performance_from_begin( &t_org, &t_new );
				const Performance *p2 = pm.performance_from_checkpoint( &t_cur, &t_new );
				fprintf( stdout, "Performance from begin ( %ld - %ld ) elapsed %ld second\n", t_org, t_new, t_new - t_org );
				p1->dump( stdout, (double)t_new - t_org );
				fprintf( stderr, "Performance from checkpoint ( %ld - %ld ) elapsed %ld second\n", t_cur, t_new, t_new - t_cur );
				p2->dump( stderr, (double)t_new - t_cur );
			}
			sleep(1);
		}
	}
	return 0;
}

void usage()
{
	puts("Usage: dbtool [-h|--help] [-r|--rebuild] [-c|--collection] [-m|--monitor] .... ");
	exit(0);
}

int main(int argc, char *argv[])
{
	option options[] = 
	{ 
		{ "help",        no_argument,       NULL, 'h' },
		{ "rebuild",     no_argument,       NULL, 'r' },
		{ "collection",  no_argument,       NULL, 'c' },
		{ "monitor",     no_argument,       NULL, 'm' },
		NULL
	};

	for ( int n = 0, c; (c = getopt_long( argc, argv, "hrcm", options, &n )) != -1; )
	{
		switch ( c )
		{
		case 'r': return rebuild_main( argc, argv);
		case 'c': return dbcollection_main( argc, argv);
		case 'm': return monitor_main( argc, argv);
		default : usage();
		}
	}
	usage();
	return 0;
}

