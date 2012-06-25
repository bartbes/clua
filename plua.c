// stdlib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// lua
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

// physfs
#include <physfs.h>

int loader_fail(lua_State *L)
{
	// We haven't found anything, return this
	lua_pushstring(L, "\n\tNot found in archive.");
	return 1;
}

int loader(lua_State *L)
{
	const char *module_name = lua_tostring(L, 1);
	size_t len = strlen(module_name);

	// Create a copy we can work
	char *tokenized = (char*) malloc(len+10);
	strcpy(tokenized, module_name);
	tokenized[len] = '\0';

	// Replace all periods by the folder separator
	int i;
	for (i = 0; i < len; i++)
	{
		if (tokenized[i] == '.')
			tokenized[i] = '/';
	}

	// If it's a folder, we need /init.lua
	if (PHYSFS_isDirectory(tokenized))
		strcpy(tokenized+len, "/init.lua");
	// Otherwise suffix .lua
	else
		strcpy(tokenized+len, ".lua");

	// If it doesn't exist, or it's not a file, bail
	if (!PHYSFS_exists(tokenized) || PHYSFS_isDirectory(tokenized))
	{
		free(tokenized);
		return loader_fail(L);
	}

	// Try opening it
	PHYSFS_File *handle = PHYSFS_openRead(tokenized);

	// We can't open it, bail
	if (!handle)
	{
		free(tokenized);
		return loader_fail(L);
	}

	// Get the file length, prepare a string for the contents
	size_t filelen = (size_t) PHYSFS_fileLength(handle);
	char *contents = (char*) malloc(filelen+1);
	contents[filelen] = '\0';

	// Perform the actual read
	size_t read = (size_t) PHYSFS_read(handle, contents, 1, filelen);

	// Not a full read? Bail!
	if (read < filelen)
	{
		free(tokenized);
		free(contents);
		return loader_fail(L);
	}

	// Let's pass it to lua
	if (luaL_loadbuffer(L, contents, filelen, tokenized) != 0)
	{
		// There's an error? Pass it on!
		free(tokenized);
		free(contents);
		return lua_error(L);
	}

	// Alright, so we found it,
	// read it, loaded it,
	// let's clean up and return it
	free(tokenized);
	free(contents);

	return 1;
}

// Expects:
// - table
// - value
// Prepends that value to the table
// (numerically indexed), then
// leaves the table on the stack
void prepend(lua_State *L)
{
	// Get the length of the table,
	// for iteration
	size_t length = lua_objlen(L, -2);

	// Move everything up once
	size_t i;
	for (i = length; i > 0; i--)
	{
		lua_rawgeti(L, -2, i);
		lua_rawseti(L, -3, i+1);
	}

	// Insert our value at 1
	lua_rawseti(L, -2, 1);
}

int main(int argc, char **argv)
{
	// Initialize PhysicsFS
	if (!PHYSFS_init(argv[0]))
		return 1;

	// Mount ourselves, if possible
	if (!PHYSFS_mount(argv[0], "/", 0))
	{
		fprintf(stderr, "Could not mount self (at %s)\n", argv[0]);
		return 2;
	}

	// Create a lua state
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	// Register our loader
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaders");
	lua_pushcfunction(L, loader);
	prepend(L);
	lua_pop(L, 2);

	// Try require("."), aka, load,
	// and run init.lua
	lua_getglobal(L, "require");
	lua_pushstring(L, ".");
	if (lua_pcall(L, 1, LUA_MULTRET, 0) != 0)
	{
		const char *message = lua_tostring(L, -1);
		fprintf(stderr, "Error: %s\n", message);
		return 3;
	}

	// If we have a return value,
	// return that later on
	int retval = 0;
	if (lua_isnumber(L, -1))
		retval = lua_tonumber(L, -1);

	// Close up lua's state
	lua_close(L);

	// De-initialize PhysicsFS
	PHYSFS_deinit();

	return retval;
}
