# 4cast

## Installation

Make sure to run **install.sh** *before* you run the binary:

```
sh install.sh && make
```
Hopefully I'll learn how to write an actual install script soon. For now, we
just have this...

You may need to run the binary twice for the data files to be properly updated
. Don't be worried if all the values are null/0 the first time you run
printscreen.

## About

4cast is a work-in-progress TUI weather app written in C and zsh script, held
together by duct tape and sheer will. It uses tools such as `curl` and the
ncurses library to retrieve data from various free weather API services, and to
display it as a forecast on your console. 

This program is inspired by [wego](https://github.com/schachmat/wego) and its
amazing command-line forecast, [wttr.in](https://github.com/chubin/wttr.in). I
highly recommend checking them out. 

The to-dos with the most priority are:

- Modify the function `fill_fc` to allow for dynamic handling of string length
- Add wind direction and wind speed to display
- Allow for user input of location (though this would require some sort of 
database storage and serialization, perhaps in csv format?)
- Allow for toggling between Fahrenheit and Celsius (via ncurses menu?)
- Implement color (for terminals that have full 256 display)
- Implement scrolling and interactive effects of cursor hovering


