Abombniball - Version 0.2b (C) 2001
Martin Donlon <akawaka@skynet.ie>

History
-------
Abombniball is based on a game of the same name which featured on the
coverdisk of ST Format several years ago. I don't know whether
Abombniball was released in any other forum, and a Google search
produces only vague references. Anyway, I felt Abombniball was too good
a game to simply vanish and with exams looming I needed a distraction
from study so I did a remake. After three days I had a fully playable
and after another days work version 0.1 was released.

With exams only 4 days away I thought I should write a README too:)


Requirements
------------
A compiler
An Operating System, Linux is what this was designed for. However it has
been successfully compiled on MacOS, Windows and Solaris.
You also need SDL >1.1.0, and matching SDL_mixer and SDL_image
libraries.
2 eggs
1 tblspoon of baking soda
and
Some smokes



Building
--------
Building (on unix at least) follows the usual "./configure; make"
mantra. Don't do a "make install", I haven't sorted that out yet, just
run "./abombniball" from the src/ directory.



Playing
-------
The objective of Abombniball is to defuse all the explosives on each
level. As a ball, this would normally be a simple task, however each
level is filled with traps and devious puzzles placed there
by...oh...lets say "Dr. Y-Front", your arch-nemesis (he's very evil).
These traps take the form of special tiles which disappear or do other
nasty things.

  Gray Tile  - A solid tile. He never falls away and abandons you like
               all those other tiles. He's always be there for you.

  Blue Tile
  Green Tile
  Red Tile   - These all vanish after you leave them. Leaving a gapping
               hole in the grid and an empty space in your heart.

  2 Tile     - More caring and forgiving than those selfish color tiles.
               You can bounce on these submissive fellas twice before
               they vanish.

  Blue Arrow - Rude fellows. Shove you in whatever direction the arrow
               is pointing.

  Red Arrow  - Even ruder still. They make you jump in whatever
               direction the arrow is pointing.

You defuse bombs by landing on the bombs square, however you can only
defuse the bomb thats currently counting down. Trying to defuse any
other bomb will cause it to explode.

Each level provides you with a certain amount of jumps, they are limit
so use them with care.



Controls
--------
  Left/Q/H             - Move left
  Right/W/L            - Move right
  Up/E/K               - Move up
  Down/R/J             - Move down

  Space/Ctrl/T/Enter   - Jump

 

Stolen Stuff
------------
Rapid development more often involves taking a few short-cuts. The font
is taken from Do'SSi Zo'la (http://dossizola.sourceforge.net), which is
GPL so there isn't really a problem there and I have contacted the
authors. The music was downloaded from www.chiptune.com and I'm not sure
what license it is under, I am attempting to contact the authors and the
music files are not included under the GPL license that rest of this
package is under. Finally, the idea is stolen. If the original author of
Abombniball ever reads this then please contact me.


