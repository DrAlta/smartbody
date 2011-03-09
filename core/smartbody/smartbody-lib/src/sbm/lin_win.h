#ifndef LIN_WIN_PORTABILITY_H
#define LIN_WIN_PORTABILITY_H

#ifdef WIN32
#else
#endif

int LinWin_strcmp( const char *s1, const char *s2 );
int LinWin_strncmp( const char *s1, const char *s2, int n );

#endif
