CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++11
INCLUDES = -I../../incs -I.

PROJECT_CXXFLAGS = -Wall -Wextra -Werror -std=c++98

PROJECT_ROOT = ../..
OBJ_DIR = ../../obj
TEST_OBJ_DIR = obj
STEP_DEFS_DIR = step_definitions
SUPPORT_DIR = support

TEST_RUNNER = test_runner

TEST_SOURCES = main.cpp \
               $(STEP_DEFS_DIR)/authentication_steps.cpp \
               $(STEP_DEFS_DIR)/message_steps.cpp \
               $(STEP_DEFS_DIR)/channel_steps.cpp \
               $(SUPPORT_DIR)/test_helpers.cpp

TEST_OBJS = $(patsubst %.cpp,$(TEST_OBJ_DIR)/%.o,$(TEST_SOURCES))

PROJECT_OBJS = $(OBJ_DIR)/srcs/server.o \
               $(OBJ_DIR)/srcs/client.o \
               $(OBJ_DIR)/srcs/channel.o \
               $(OBJ_DIR)/srcs/parsing.o \
               $(OBJ_DIR)/srcs/DccServer.o \
               $(OBJ_DIR)/srcs/DccClient.o \
               $(OBJ_DIR)/srcs/utils/prompt.o \
               $(OBJ_DIR)/srcs/commands/PassCommand.o \
               $(OBJ_DIR)/srcs/commands/NickCommand.o \
               $(OBJ_DIR)/srcs/commands/UserCommand.o \
               $(OBJ_DIR)/srcs/commands/JoinCommand.o \
               $(OBJ_DIR)/srcs/commands/PartCommand.o \
               $(OBJ_DIR)/srcs/commands/KickCommand.o \
               $(OBJ_DIR)/srcs/commands/InviteCommand.o \
               $(OBJ_DIR)/srcs/commands/TopicCommand.o \
               $(OBJ_DIR)/srcs/commands/ModeCommand.o \
               $(OBJ_DIR)/srcs/commands/PrivmsgCommand.o

all: $(TEST_RUNNER)

build-project:
	@echo "Compilando projeto principal..."
	@cd $(PROJECT_ROOT) && $(MAKE)

$(TEST_RUNNER): build-project $(TEST_OBJS)
	@echo "Linkando testes..."
	$(CXX) $(CXXFLAGS) $(TEST_OBJS) $(PROJECT_OBJS) -o $(TEST_RUNNER)
	@echo "✓ Testes compilados com sucesso: $(TEST_RUNNER)"

$(TEST_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compilando teste: $<"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

test: $(TEST_RUNNER)
	@echo ""
	@echo "================================"
	@echo "   Executando Testes BDD"
	@echo "================================"
	@echo ""
	./$(TEST_RUNNER)
	@echo ""
	@echo "================================"
	@echo "   Testes Concluídos"
	@echo "================================"


test-valgrind: $(TEST_RUNNER)
	@echo ""
	@echo "================================"
	@echo "   Testes BDD com Valgrind"
	@echo "================================"
	@bash test_valgrind_bdd.sh

clean:
	rm -rf $(TEST_OBJ_DIR)
	@echo "✓ Objetos de teste removidos"

fclean: clean
	rm -f $(TEST_RUNNER)
	@echo "✓ Executável de teste removido"

re: fclean all

.PHONY: all test clean fclean re build-project help test-valgrind