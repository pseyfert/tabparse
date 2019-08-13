# tabparse

Sure, there are many many command line parsers around and we don't need more.
What I wanted to explore is, if I can make a command line parser, that
integrates better with tabcompletion systems in the following aspects:

 - auto generated: A manually written tab completion function can (in my
   experience) easily be better than one generated by `_gnu_generic`. However,
   this introduces a maintenance burden, easily on a maintainer that doesn't
   know how to write tabcompletion functions, or doesn't bother, or simply
   would prefer not to invest time in its maintenance.

 - call appropriate completion helper functions: Some command line parsing
   frameworks I've seen already integrate into tabcompletions but fall short
   of exposing to the completion system if an argument string is just a string
   or the name of a directory or the name of a file with a certain suffix, or
   from a list of string options (output format).

Clearly, these features only really benefit us, if adopted by many applications
out there. Ideally developers unaware of tabcompletion systems just write their
applications, and any user or package maintainer can generate the completion
function through the existing functionality of the parser. I doubt the
repository here will find wide adoption in the world out there. I consider it
an experiment.
