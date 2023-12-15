# Compiled Files
NAME	=	webserv

# Compiler and flags
CCXX		=	c++
CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98 -g -I includes #-fstandalone-debug

# Directories
SRC_DIR = src
BIN_DIR   = bin

# Files
SRCS := \
	$(SRC_DIR)/config/ConfigParser.cpp \
	$(SRC_DIR)/config/ServerConfig.cpp \
	$(SRC_DIR)/config/Location.cpp \
	$(SRC_DIR)/core/HttpServer.cpp \
	$(SRC_DIR)/core/HttpRequestHandler.cpp \
	$(SRC_DIR)/core/HttpResponse.cpp \
	$(SRC_DIR)/core/HttpRequest.cpp \
	$(SRC_DIR)/core/HttpStatusCode.cpp \
	$(SRC_DIR)/core/RouteHandler.cpp \
	$(SRC_DIR)/core/Utils.cpp \
	$(SRC_DIR)/main.cpp
	#$(SRC_DIR)/cgi/CgiHandler.cpp \

OBJS	=	$(patsubst $(SRC_DIR)/%.cpp, $(BIN_DIR)/%.o, $(SRCS))
ARGS	= 	

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

test:
		@echo "Testing server with valid request..."
		curl -X GET "http://localhost:8080/resource" -H "Host: example.com"

		@echo "Testing server with invalid resource..."
		curl -X GET "http://localhost:8080/lol" -H "Host: example.com"

		@echo "\nTesting server with POST request..."
		curl -X POST "http://localhost:8080/resource?param1=value1&param2=value2" -H "Host: example.com"

		@echo "\nTesting server with invalid request (Bad method)..."
		curl -X INVALIDMETHOD "http://localhost:8080/resource" -H "Host: example.com"

		@echo "\nTesting server with invalid request (No Host header)..."
		printf "GET /resource HTTP/1.1\r\n\r\n" | nc localhost 8080

		@echo "\nTesting server with invalid request (No method)..."
		printf "/resource HTTP/1.1\r\nHost: example.com\r\n\r\n" | nc localhost 8080

		@echo "\nTesting server with invalid request (No HTTP version)..."
		printf "GET /resource\r\nHost: example.com\r\n\r\n" | nc localhost 8080

		@echo "\nTesting server with invalid request (Invalid HTTP version)..."
		printf "GET /resource HTTP/2.0\r\nHost: example.com\r\n\r\n" | nc localhost 8080

		@echo "\nTesting server with invalid request (Empty request)..."
		printf "\r\n" | nc localhost 8080

		@echo "\nTesting server with invalid request (Invalid URL)..."
		curl -X GET "http://localhost:8080/resource?param1=value1&param2" -H "Host: example.com"

		@echo "Testing server using GET to retrieve a file"
		curl -X GET "http://localhost:8080/resource/file1.txt" -H "Host: example.com"

		@echo "Testing POST file upload to server"
		curl -X POST http://localhost:8080/resource -F "file=@YoupiBanane/file12.txt"

		@echo "Testing POST text upload to server"
		curl -X POST http://localhost:8080/resource -d "example of body"

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