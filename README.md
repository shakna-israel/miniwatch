# miniwatch

A tiny file watcher for Linux

---

## Usage

	miniwatch FILE_TO_WATCH -c ARG ARG etc

miniwatch will watch for the modified time of the given file, and when it detects a change in the modified time, it will run the given command and arguments.

For example:

	miniwatch src/main.c -c gcc -o miniwatch src/main.c

There are some caveats:

+ If the given file to watch is actually a directory, some changes will be noticed and reacted to, but not all. This isn't a strong suit of `miniwatch`. Use something else.

+ Because `stat` will peg one core of your CPU if it blindly runs in a loop, waiting for changes, `miniwatch` actually sleeps for a full second before checking the file each time.

+ `miniwatch` assumes that time moves forward, and modified times will result in similar linear behaviour. This is naive in the extreme, but should work most of the time.

+ `miniwatch` won't drop into the background whilst it is running. I like it this way, you might not. `nohup` is your friend.

+ There is minimal, absolutely minimal, safety around the CLI parsing. The order of arguments matters.

+ Command errors are ignored.

---

## Why?

I wanted a program to watch and react to changes, but didn't want to install something like `inotify`.

So I spent a whopping five minutes building this.

An explanation of the pathetically minimal code can be found [here](https://shatterealm.netlify.app/programming/2021_03_23_lets_build_a_file_watcher) if you're thinking of making it more robust.

---

# License

See the `LICENSE` file for the legally binding text.

CC0 1.0 Universal at time of writing.
