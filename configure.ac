AC_INIT([clua], [tip])
AC_CONFIG_SRCDIR([.])
AM_INIT_AUTOMAKE([1.10 foreign silent-rules])
AM_SILENT_RULES([yes])
AC_PROG_CC
lua=lua
AC_ARG_WITH([luajit],
	    [AS_HELP_STRING([--with-luajit], [Use LuaJIT instead of lua and llvm-lua])],
	    [lua=luajit],
	    [])
AC_ARG_WITH([llvm-lua],
	    [AS_HELP_STRING([--with-llvm-lua], [Use llvm-lua instead of lua and LuaJIT])],
	    [lua=llvm-lua],
	    [])

AS_IF([test "$lua" = "lua"],
	AC_SEARCH_LIBS(
		[lua_pcall],
		[lua lua5.1],
		if test "$ac_cv_search_lua_pcall" = "-llua5.1"; then
			AC_SUBST([INCLUDE_LUA], [-I/usr/include/lua5.1])
		fi,
		AC_MSG_ERROR([Can't find Lua])
	)
      )
AS_IF([test "$lua" = "luajit"],
	AC_SEARCH_LIBS(
		[lua_pcall],
		[luajit luajit-5.1],
		AC_SUBST([INCLUDE_LUA], [-I/usr/include/luajit-2.0]),
		AC_MSG_ERROR([Can't find LuaJIT])
	)
      )
AS_IF([test "$lua" = "llvm-lua"],
	AC_SEARCH_LIBS(
		[lua_pcall],
		[llvm-lua],
		[],
		AC_MSG_ERROR([Can't find llvm-lua])
	)
      )

AC_SEARCH_LIBS([PHYSFS_init], [physfs], [], AC_MSG_ERROR([Can't find PhysicsFS]))

AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT
