local L = require 'linenoise'
L.clear_screen()
print '----- Testing lua-linenoise! ------'
local prompt, history = '? ', 'history.txt'
L.load_history(history) -- load existing history
L.set_completion(function(c,s)
   if s == 'h' then
    c:add('help') -- same as L.addcompletion(c,'help)
    L.add_completion(c,'halt') -- same as c:add('halt')
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
