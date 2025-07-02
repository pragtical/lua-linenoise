# lua-linenoise - Lua binding for the linenoise command line library

Fork of https://github.com/hoelzro/lua-linenoise for Pragtical adjusted to use
https://github.com/arangodb/linenoise-ng which has proper Windows support.

# Compilation

meson setup build
meson compile -C build

# Usage

This library is a fairly thin wrapper over linenoise itself, so the function calls
are named similarly.  I may develop a "porcelain" layer in the future.

## L.linenoise(prompt)

Prompts for a line of input, using *prompt* as the prompt string.  Returns nil if
no more input is available; Returns nil and an error string if an error occurred.

## L.historyadd(line)

Adds *line* to the history list.

## L.historysetmaxlen(length)

Sets the history list size to *length*.

## L.historysave(filename)

Saves the history list to *filename*.

## L.historyload(filename)

Loads the history list from *filename*.

## L.clearscreen()

Clears the screen.

## L.setcompletion(callback)

Sets the completion callback.  This callback is called with two arguments:

  * A completions object.  Use object:add or L.addcompletion to add a completion to this object.
  * The current line of input.

## L.addcompletion(completions, string)

Adds *string* to the list of completions.

All functions return nil on error; functions that don't have an obvious return value
return true on success.

## L.setmultiline(multiline)

Enables multi-line mode if *multiline* is true, disables otherwise.

## L.printkeycodes()

Prints linenoise key codes.  Primarly used for debugging.

# Example

```lua
local L = require 'linenoise'
L.clearscreen()
print '----- Testing lua-linenoise! ------'
local prompt, history = '? ', 'history.txt'
L.historyload(history) -- load existing history
L.setcompletion(function(completion,str)
   if str == 'h' then
    completion:add('help')
    completion:add('halt')
  end
end)

local line, err = L.linenoise(prompt)
while line do
    if #line > 0 then
        print(line:upper())
        L.historyadd(line)
        L.historysave(history) -- save every new line
    end
    line, err = L.linenoise(prompt)
end
if err then
  print('An error occurred: ' .. err)
end
```
