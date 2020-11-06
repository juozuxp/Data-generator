The script revolves around itself being the final file,  the code can be merged with characters of almost any sort,  code will execute leaving no mark of it ever being there or it will replace itself with the result.<br>
Example: 
<RR=[10, 100, 10]>; <RR=[10, 100, 10]>  potential result can be: 20;50
The execution file is always named “Settings.txt” and will be grabbed from the execution folder.
Most of the code will revolve around these symbols: < >  [ ] = “ , \
< > - Function region symbols
[ ] – Argument start and stop symbols
= - Argument listing symbol
“ – String marking symbol
, - Argument separating symbol
\ - Blocker symbol
They will all contribute to code execution and formatting unless prefixed with the blocking symbol
Example:
<RA=[“Hello \”It’s me\””, “Even though \“I’m here\””]> potential result can be: Hello “It’s me”
The code as of now is case sensitive, and spacing insensitive.
When it comes to functions, they are executed orderly (one after another) there are 2 main types that will be used integers and strings, strings must be prefixed and sufixed with " symbols (there are implementations of a floating point type but that’s reserved for the future) they are passed through arguments which come in 4 forms, single typed, multi typed, optional and variadic – infinite.
Currently supported functions:
SI – Set integer, sets an integer variable array. Arguments: [String ArrayName, Variadic Integer Variables]
SS – Set string, sets a string variable array. Arguments: [String ArrayName, Variadic String Variables]
SF – Set File, reads file data, separates it and stores it inside a variable array. Arguments: [String ArrayName, Optional Variadic String Seperaters (default: \r\n)]
RR – Random range, generates a value in a range and replaces itself with the result. Arguments: [String(Alphabetical)/Integer StartRange, String(Alphabetical)/Integer EndRange, Optional Integer Step (default: 1)]
RA – Random Array, grabs an array element from a random index. Arguments: [Variadic String Array]
SP – Spawn, generates lines of text replacing itself (this function can spawn code and will restart the execution). Arguments: [String Text, Integer AmountToSpawn]
OF – Output File, outputs the result into a file. Arguments: [String FileName]
CB – Clear Breaks, clears access line breaks and spaces.
DF – Display File, opens a file post execution.  Arguments: [String FilePath(FileName)]
DD – Display Directory, opens a directory post execution. Arguments: [Optional String DirectoryPath(DirectoryName, default: Current Directory)]
The code contains variable arrays, they are both typed and type less (e.g. a integer can both be an integer and a string or a string can both be an integer (if parse able) and a string) variables can be used as arguments and don’t require any prefixing.
Example:
<SS=[“Random”, ”0”, “100”, “10”]>
<RR=[Random]> possible result can be 50
Spawn and Set file function will replace blocked symbols with their counterparts and have unique symbols which will be replaced with their corresponding equivalence:
\n – Line break
\r – Carriage return
\\ - \
\< - <
\> - >
\” – “
\, - ,
For an example refer to “Settings Example” folder.
Currently there is no error policy, all code errors will probably result in an exception which will crash the parser, zero error tolerance will come with the future updates.
