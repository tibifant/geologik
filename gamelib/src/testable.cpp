#include "testable.h"

#include <map>

static std::map<std::string, testable_func> *_pTests;

_testable_init register_testable(const char *name, testable_func func)
{
  static bool initialized = false;

  if (!initialized)
  {
    initialized = true;
    _pTests = new std::map<std::string, testable_func>();
  }

  _pTests->insert(std::make_pair(name, func));

  return { _pTests->size() };
}

lsResult run_testables()
{
  register_testable_files<10>(); // <-- INCREMENT, when new tests are added.

  lsResult result = lsR_Success;

  lsErrorPushSilentImpl _silent;
  size_t failed = 0;
  size_t succeeded = 0;

  if (_pTests == nullptr)
  {
    print_error_line("No tests discovered.");
    goto epilogue;
  }

  print(_pTests->size(), " tests discovered.\n\n");

  for (const auto &_item : *_pTests)
  {
    lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
    print("[RUNNING] ");
    lsResetConsoleColor();
    print(_item.first.c_str(), "\n");

    const lsResult r = _item.second();

    if (LS_FAILED(r))
    {
      failed++;
      result = lsR_Failure;
      lsSetConsoleColor(lsCC_BrightRed, lsCC_Black);
      print("[XFAILED] ", _item.first.c_str(), "\n");
      lsResetConsoleColor();
    }
    else
    {
      succeeded++;
      lsSetConsoleColor(lsCC_BrightGreen, lsCC_Black);
      print("[SUCCESS] ");
      lsResetConsoleColor();
      print(_item.first.c_str(), "\n");
    }
  }

  print("===========================================\n");
  
  if (succeeded > 0)
  {
    lsSetConsoleColor(lsCC_BrightGreen, lsCC_Black);
    print(succeeded, " / ", _pTests->size(), " (", FD(Max(4))(succeeded / (double_t)_pTests->size() * 100.0), " %) Tests succeeded.\n");
    lsResetConsoleColor();
  }
  
  if (failed > 0)
  {
    lsSetConsoleColor(lsCC_BrightRed, lsCC_Black);
    print(failed, " / ", _pTests->size(), " (",  FD(Max(4))(failed / (double_t)_pTests->size() * 100.0), " %) Tests failed.\n");
    lsResetConsoleColor();
  }

  print("===========================================\n");

  goto epilogue;
epilogue:
  return result;
}
