# Compiled Files
NAME	=	webserv

# Compiler and flags
CCXX		=	c++
CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98 -g -I includes

# Directories
SRC_DIR = src
BIN_DIR   = bin

# Files
SRCS := \
    $(SRC_DIR)/cgi/CgiHandler.cpp \
    $(SRC_DIR)/config/ConfigParser.cpp \
	$(SRC_DIR)/config/ServerConfig.cpp \
    $(SRC_DIR)/core/HttpServer.cpp \
    $(SRC_DIR)/core/HttpRequestHandler.cpp \
	$(SRC_DIR)/core/HttpResponse.cpp \
	$(SRC_DIR)/core/HttpRequest.cpp \
    $(SRC_DIR)/main.cpp

OBJS	=	$(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SRCS))
ARGS	= 	""

# Colors

DEFAULT = \033[0;39m
GRAY 	= \033[0;90m
RED 	= \033[0;91m
GREEN 	= \033[0;92m
YELLOW 	= \033[0;93m
BLUE 	= \033[0;94m
MAGENTA = \033[0;95m
CYAN 	= \033[0;96m
WHITE 	= \033[0;97m
CURSIVE	= \e[33;3m


all:		$(NAME)

$(NAME): $(OBJS)
			@echo "$(CURSIVE)Compiling...$(DEFAULT)"
			$(CCXX) $(CXXFLAGS) $(OBJS) -g -o $(NAME)
			@echo "$(GREEN)webserv created successfully!$(DEFAULT)"

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp
			@mkdir -p $(dir $@)
			$(CCXX) $(CXXFLAGS) -c $< -o $@

clean:
			@echo "$(BLUE)Cleaning...$(DEFAULT)"
			@rm -rf $(BIN_DIR)
			@echo "$(CYAN)Object Files Cleaned!$(DEFAULT)"

fclean:		clean
			@rm -f $(NAME)
			@echo "$(BLUE)Executables Cleaned!$(DEFAULT)"

re:			fclean all
			@echo "$(MAGENTA)Cleaned and rebuilt!$(DEFAULT)"


run: all
	 ./$(NAME) $(ARGS)

rerun: re run

gdb:	all
		gdb --args ./$(NAME) $(ARGS)

lldb:	all
		lldb ./$(NAME) $(ARGS)

valgrind: 	all
			valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) $(ARGS)

.PHONY: re all clean fclean debug test $(LIBFT)