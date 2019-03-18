# EncodingParser
A light version of PowerShell Format command. Originally used as a UTF-8 parser, it is considered to add support for more encodings and more display options.

Execute the application providing an absolute path via a command line argument.

The application will then display decimal values for each UTF-8 character, or report errors if the file is corrupted.

# Requirements:

- boost for boost::format;
- C++17, but the code can be altered easily to work with C++11/14;
