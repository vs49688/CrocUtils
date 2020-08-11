#include <string.h>
#include <libcroc.h>

const char *croc_util_get_filename(const char *path)
{
	const char *start = strrchr(path, '/');
	if(start == NULL)
		start = strrchr(path, '\\');

	if(start == NULL)
		return path;
	else
		return start + 1;
}


/* Is the file a Saturn MPLOAD%02u.WAD? */
int croc_wad_is_mpload(const char *path, unsigned int *level)
{
	unsigned int _level;
	int pos;
	char c;
	const char *filename = croc_util_get_filename(path);

	if(sscanf(filename, "%*1[mM]%*1[pP]%*1[lL]%*1[oO]%*1[aA]%*1[dD]%2u.%*1[wW]%*1[aA]%c%n", &_level, &c, &pos) != 2)
		return 0;

	/* sscanf doesn't like that last "%1[dD] for some reason. */
	if(c != 'd' && c != 'D')
		return 0;

	if(filename[pos] != '\0')
		return 0;

	if(level != NULL)
		*level = _level;

	return 1;
}

/* Is the file a PSX MAP%02u.WAD? */
int croc_wad_is_mapxx(const char *path, unsigned int *level)
{
	unsigned int _level;
	int pos;
	char c;
	const char *filename = croc_util_get_filename(path);

	if(sscanf(filename, "%*1[mM]%*1[aA]%*1[pP]%2u.%*1[wW]%*1[aA]%c%n", &_level, &c, &pos) != 2)
		return 0;

	/* sscanf doesn't like that last "%1[dD] for some reason. */
	if(c != 'd' && c != 'D')
		return 0;

	if(filename[pos] != '\0')
		return 0;

	if(level != NULL)
		*level = _level;

	return 1;
}
