The script revolves around itself being the final file,  the code can be merged with characters of almost any sort,  code will execute leaving no mark of it ever being there or it will replace itself with the result.<br>
Example: <br>
<RR=[10, 100, 10]>; <RR=[10, 100, 10]>  potential result can be: 20;50<br>
The execution file is always named “Settings.txt” and will be grabbed from the execution folder.<br>
Most of the code will revolve around these symbols: < >  [ ] = “ , \ <br>
< > - Function region symbols<br>
[ ] – Argument start and stop symbols<br>
= - Argument listing symbol<br>
“ – String marking symbol<br>
, - Argument separating symbol<br>
\ - Blocker symbol<br>
They will all contribute to code execution and formatting unless prefixed with the blocking symbol<br>
Example:<br>
<RA=[“Hello \”It’s me\””, “Even though \“I’m here\””]> potential result can be: Hello “It’s me”<br>
The code as of now is case sensitive, and spacing insensitive.<br>
When it comes to functions, they are executed orderly (one after another) there are 2 main types that will be used integers and strings, strings must be prefixed and sufixed<br> with " symbols (there are implementations of a floating point type but that’s reserved for the future) they are passed through arguments which come in 4 forms, single typed,<br> multi typed, optional and variadic – infinite.<br>
Currently supported functions:<br>
SI – Set integer, sets an integer variable array. Arguments: [String ArrayName, Variadic Integer Variables]<br>
SS – Set string, sets a string variable array. Arguments: [String ArrayName, Variadic String Variables]<br>
SF – Set File, reads file data, separates it and stores it inside a variable array. Arguments: [String ArrayName, Optional Variadic String Seperaters (default: \r\n)]<br>
RR – Random range, generates a value in a range and replaces itself with the result. Arguments: [String(Alphabetical)/Integer StartRange, String(Alphabetical)/Integer EndRange, Optional Integer Step (default: 1)]<br>
RA – Random Array, grabs an array element from a random index. Arguments: [Variadic String Array]<br>
SP – Spawn, generates lines of text replacing itself. Arguments: [String Text, Integer AmountToSpawn]<br>
OF – Output File, outputs the result into a file. Arguments: [String FileName]<br>
CB – Clear Breaks, clears access line breaks and spaces.<br>
RS - Restart Execution, starts executing from the start
DF – Display File, opens a file post execution.  Arguments: [String FilePath(FileName)]<br>
DD – Display Directory, opens a directory post execution. Arguments: [Optional String DirectoryPath(DirectoryName, default: Current Directory)]<br>
All functions can contain code and spawn it if they're designed to.
The code contains variable arrays, they are both typed and type less (e.g. a integer can both be an integer and a string or a string can both be an integer (if parse able) and a string) variables can be used as arguments and don’t require any prefixing.<br>
Example:<br>
<SS=[“Random”, ”0”, “100”, “10”]><br>
<RR=[Random]> possible result can be 50<br>
Spawn and Set file function will replace blocked symbols with their counterparts and have unique symbols which will be replaced with their corresponding equivalence:<br>
\n – Line break<br>
\r – Carriage return<br>
\\\ - \ <br>
\\< - <<br>
\\> - ><br>
\” – “<br>
\\, - ,<br>
For an example refer to “Settings Example” folder.<br>
