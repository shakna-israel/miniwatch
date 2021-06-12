# miniwatch

A tiny file watcher for Linux

---

## Usage

	miniwatch FILE_OR_DIR_TO_WATCH -c ARG ARG etc

miniwatch will watch for the modified time of the given file, and when it detects a change in the modified time, it will run the given command and arguments.

If it was given a directory, it will also watch for file creation & deletion to trigger the command. (Directories are checked recursively for files).

For example:

	miniwatch src/main.c -c gcc -o miniwatch src/main.c

There are some caveats:

+ Because `stat` will peg one core of your CPU if it blindly runs in a loop, waiting for changes, `miniwatch` actually sleeps for a full second before checking the file/directory each time.

+ `miniwatch` assumes that time moves forward, and modified times will result in similar linear behaviour. This is naive in the extreme, but should work most of the time.

+ `miniwatch` won't drop into the background whilst it is running. I like it this way, you might not. `nohup` is your friend.

+ There is minimal, absolutely minimal, safety around the CLI parsing. The order of arguments matters.

+ Command errors are ignored.

---

## Why?

I wanted a program to watch and react to changes, but didn't want to install something like `inotify`.

So I spent a whopping five minutes building the initial version of this:

An explanation of the pathetically minimal code can be found [here](https://shatterealm.netlify.app/programming/2021_03_23_lets_build_a_file_watcher) if you're thinking of making it more robust.

Update: The directory handling code is a little bit more verbose than what is explained above, using a basic key-value store. It is still drop-dead simple. It took me a further hour to implement, and isn't really handling errors, either.

---

# License

See the `LICENSE` file for the legally binding text.

CC0 1.0 Universal at time of writing.
