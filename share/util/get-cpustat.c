
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define STAT_FILE	"/proc/stat"

static void
getstat(int64_t *cuse, int64_t *cice, int64_t *csys, int64_t *cide, int64_t *ciow, int64_t *cirq, int64_t *csoft)
{
	int			statfd;
	char		buff[1024];
	int			bsize = 0;
	char		*b;

	*cuse = *cice = *csys = *cide = *ciow = *cirq = *csoft = 0;

	if ((statfd = open(STAT_FILE, O_RDONLY, 0)) != -1)
	{
		bsize = read(statfd, buff, sizeof(buff));
		close(statfd);

		if( bsize > 0 )
		{
			buff[bsize-1] = '\n';
			if( b = strstr(buff, "cpu ") )
			{
				if(sscanf(b,"cpu  %llu %llu %llu %llu %llu %llu %llu",
						cuse,cice,csys,cide,ciow,cirq,csoft) != 7)
					sscanf(b, "cpu  %lu %lu %lu %lu", cuse, cice, csys, cide);
			}
		}
	}
}

int main( int argc, char ** argv )
{
	int64_t	cuse, cice, csys, cide;
	int64_t	ciow, cirq, csoft;
	getstat( &cuse, &cice, &csys, &cide, &ciow, &cirq, &csoft );
	printf( "%llu\n%llu\n%llu\n%llu\n", cuse + cice, ciow+cirq+csoft, csys, cide );
}

