# ifndef _GOOGLE_TRANSLATE_API_CLIENT_
# define _GOOGLE_TRANSLATE_API_CLIENT_

void init_client();
void cleanup_client();
char *translate_text(char *text, char *from, char *to);

# endif