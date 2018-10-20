namespace os
{
static const Char env_path_sep =
#ifdef _WIN32
u';'
#else
u':'
#endif
;
}
