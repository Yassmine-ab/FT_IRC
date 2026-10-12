################################################################################
#                                     COLORS                                   #
################################################################################

DEFAULT			= \033[0m
RED				= \033[1;31m
GREEN			= \033[1;32m
YELLOW			= \033[1;33m
BLUE			= \033[1;34m
MAGENTA			= \033[1;35m
CYAN			= \033[1;36m
WHITE			= \033[1;37m

################################################################################
#                                 PROGRESS BAR                                 #
################################################################################

define PROGRESS_BAR
	@TOTAL_STEPS=20; CURRENT_STEP=0; \
	while [ $$CURRENT_STEP -lt $$TOTAL_STEPS ]; do \
		CURRENT_STEP=$$(($$CURRENT_STEP + 1)); \
		echo -n "$(GREEN)▰$(DEFAULT)"; \
		sleep 0.01; \
	done; \
	echo " ✔️";
endef

################################################################################
#                                     CONFIG                                   #
################################################################################

NAME =			ircserv
CC =			c++
CFLAGS =		-Wall -Wextra -Werror -std=c++98
RM =			rm -f

################################################################################
#                                PROGRAM'S SOURCES                             #
################################################################################

# Directory paths
SRC =			./srcs/
OBJ =			./objs/
INC =			./includes

# Source files
SRCS =			$(SRC)main.cpp \
				$(SRC)Server.cpp \
				$(SRC)Client.cpp \
				$(SRC)Channel.cpp \
				$(SRC)CommandHandler.cpp \
				$(SRC)commands/AuthCommands.cpp \
				$(SRC)commands/ChannelCommands.cpp \
				$(SRC)commands/MessageCommands.cpp \
				$(SRC)commands/OperatorCommands.cpp

# Converts source file paths to object file paths
OBJS =			$(patsubst $(SRC)%, $(OBJ)%, $(SRCS:.cpp=.o))

################################################################################
#                                     RULES                                    #
################################################################################

# Rule for compiling source files into object files
$(OBJ)%.o:		$(SRC)%.cpp
				@mkdir -p $(dir $@)
				@$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

# Rule for creating the executable
$(NAME):		$(OBJS)
				@echo -n "\n🔗 $(WHITE)Linking $(CYAN)$(NAME)$(DEFAULT) executable\t\t\t"
				@$(CC) $(CFLAGS) -I$(INC) $(OBJS) -o $(NAME)
				$(PROGRESS_BAR)
				@echo ""

# Default rule
all:			$(NAME)

# Rule for cleaning up object files
clean:
				@echo -n "\n🧹 $(RED)Cleaning up $(CYAN)project $(DEFAULT)object files\t\t"
				@$(RM) -r $(OBJ)
				$(PROGRESS_BAR)

# Full clean rule (objects files, executable and libraries)
fclean:			clean
				@echo -n "\n🗑️  $(RED)Deleting $(CYAN)$(NAME)$(DEFAULT) executable\t\t"
				@$(RM) $(NAME)
				$(PROGRESS_BAR)
				@echo ""

# Rebuild rule
re:				fclean all

# Rule to compile the program with debugging flags
debug:			$(OBJS)
				@echo -n "\n🔗 Compiling in $(CYAN)debug$(DEFAULT) mode\t\t\t"
				@$(CC) $(CFLAGS) -I$(INC) $(OBJS) -o $(NAME) -g3 -fsanitize=address
				$(PROGRESS_BAR)

# Rule to display help
help:
				@echo "\n$(CYAN)all$(DEFAULT)		- Build the executable $(NAME)"
				@echo "$(CYAN)clean$(DEFAULT)		- Clean up object files"
				@echo "$(CYAN)fclean$(DEFAULT)		- Clean up all object files and executable"
				@echo "$(CYAN)re$(DEFAULT)		- Rebuild the entire project"
				@echo "$(CYAN)debug$(DEFAULT)		- Run the program with debugging flags -g3 -fsanitize=address\n"

# Rule to ensure that these targets are always executed as intended, even if there are files with the same name
.PHONY:			all clean fclean re debug help