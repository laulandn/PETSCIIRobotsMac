The ongoing result of a challenge on MacintoshGarden:
===============

"So let's talk about the elephant in the room. Attack of the PETSCII Robots. I can play it on my Mac - but only by using the Apple II card in my LCIII. Not the best experience possible. And PETSCII Robots has been ported to many different systems - including ones with a 68000CPU (Amiga), and ones with a black and white only screen (or, at least, black and white for the purposes of in-game graphics). The Mac is ripe for a conversion - and, given the specifications of the Mac vs those of the other computers that it's been ported to, a Mac colour version would be the most spectacular version of the game bar none."

Based on SDL version from original author.

+ Builds with CodeWarrior Pro 6 and 8.
+ Builds with XCode 2.

Supports m68k, PPC and Carbon.

+ Currently only supports 256 colors.
+ Uses raw uncompressed art assets.
+ Event handling works in menu, but not game.
+ (Keyboard repeat or something else not handled correctly maybe?)
+ Missing several important graphic functions.
+ No sound.
+ Uses 256 color art, but doesn't change palette from default!
+ (There's no Mac code to handle palettes yet)

Original SDL code is still present and works.  #define "_MAC" to build for Mac instead.

Unfinished.  

Mostly done as a proof-of-concept of how this would be a relatively simple project, even for beginning programmers.

Prints debug info to stdout (or file) so you can see which of the new Mac functions are being called.  Function names have NOT been changed, so it will still say "SDL" even for the Mac version!

Added some error checking that was missing in original.

NOTE: There are a lot of duplicate asset files, but you need to run the app with them in the same folder.  It could be cleaned up a LOT.

Feel free to contact me to help finish it, use this as a starting point for your own port, learn game coding, or simple use of the classic MacOS toolbox.
