#ifndef UTILS_HPP
# define UTILS_HPP

# include <GLFW/glfw3.h>

# include <cstring>
# include <ostream>
# include <sys/stat.h>
# include <iostream>
# include <fstream>
# include <ostream>
# include <fcntl.h>
# include <unistd.h>
# include <sstream>

# define BUFSIZE			8192

void				checkGlError(std::string file, int line);
char *				readFile(char const *filename);
float				getProb(void);
int					printError(std::ostream &msg, int const &code);
int					printError(std::string const &msg, int const &code);
void *				printError(std::string const &msg);
std::string			getFileContents(std::string const &filename);
int					stoi(const char *str);
char *				itos(int n);
int					slen(char *str);
char *				scpy(char *s1, const char *s2);

#endif