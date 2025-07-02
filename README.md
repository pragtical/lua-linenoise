# lua-linenoise - Lua binding for the linenoise command line library

Fork of https://github.com/hoelzro/lua-linenoise for Pragtical adjusted to use
https://github.com/arangodb/linenoise-ng which has proper Windows support.

# Compilation

meson setup build
meson compile -C build

# Usage

This library is a fairly thin wrapper over linenoise itself.

## L.input(prompt)

Prompts for a line of input, using *prompt* as the prompt string.  Returns nil if
no more input is available; Returns nil and an error string if an error occurred.

## L.add_history(line)

Adds *line* to the history list.

## L.set_history_max_len(length)

Sets the history list size to *length*.

## L.save_history(filename)

Saves the history list to *filename*.

## L.load_history(filename)

Loads the history list from *filename*.

## L.clear_screen()

Clears the screen.

## L.set_completion(callback)

Sets the completion callback.  This callback is called with two arguments:

  * A completions object.  Use object:add or L.addcompletion to add a completion to this object.
  * The current line of input.

## L.add_completion(completions, string)

Adds *string* to the list of completions.

All functions return nil on error; functions that don't have an obvious return value
return true on success.

## L.set_multiline(multiline)

Enables multi-line mode if *multiline* is true, disables otherwise.

## L.print_keycodes()

Prints linenoise key codes.  Primarly used for debugging.

# Example

```lua
local L = require 'linenoise'
L.clear_screen()
print '----- Testing lua-linenoise! ------'
local prompt, history = '? ', 'history.txt'
L.load_history(history) -- load existing history
L.set_completion(function(completion,str)
   if str == 'h' then
    completion:add('help')
    completion:add('halt')
  end
end)

local line, err = L.input(prompt)
while line do
    if #line > 0 then
        print(line:upper())
        L.add_history(line)
        L.save_history(history) -- save every new line
    end
    line, err = L.input(prompt)
end
if err then
  print('An error occurred: ' .. err)
end
```
