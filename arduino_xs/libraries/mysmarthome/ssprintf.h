
template <typename Arg>
struct Format
{
  static const Arg &go(const Arg &_arg) { return _arg; }
};


template <>
struct Format<bool>
{
  static const char *go(const bool &_arg) { return _arg ? "true" : "false"; }
};


template <>
struct Format<String>
{
  static const char *go(const String &_arg) { return _arg.c_str(); }
};


namespace {
  const char *ssprintfBuffer()
  {
    static char buffer[32];
    return buffer;
  }
};


template<class Output, class... Args>
void ssprintf(Output &_stream, const Args&... _args)
{
  sprintf(ssprintfBuffer(), Format<Args>::go(_args)...);
  _stream.print(ssprintfBuffer());
}

