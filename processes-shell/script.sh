#!/usr/bin/bash
# Created on: Thu Sep 18 15:43:41 +01 202

# --- Valgrind Automation Script for wish Shell ---

# 1. Configuration
EXEC_NAME="wish"
TEST_DIR="batch_tests"
LOG_DIR="valgrind_logs"

# Ensure the executable exists
if [ ! -x "./$EXEC_NAME" ]; then
    echo "ERROR: Executable './$EXEC_NAME' not found or not executable."
    echo "Attempting to compile the source file (assuming source is in wish.c)..."
    gcc -Wall -Werror -g wish.c -o wish
    if [ $? -ne 0 ]; then
        echo "Compilation failed. Exiting."
        exit 1
    fi
    echo "Compilation successful."
fi

# Create logging directory
mkdir -p $LOG_DIR
mkdir -p $TEST_DIR

echo "--- Starting Valgrind Test Suite for $EXEC_NAME ---"

# List of all test files to run
TEST_FILES=(
    "$TEST_DIR/test_01_builtins_path_misuse.txt"
    # "$TEST_DIR/test_02_redirection_parsing.txt"
    # "$TEST_DIR/test_03_parallel_mixed_errors.txt"
    # "$TEST_DIR/test_04_success_and_termination.txt"
)

# 2. Iteration and Execution
for TEST_FILE in "${TEST_FILES[@]}"; do
    # Extract basename for log file (e.g., test_01_builtins_path_misuse)
    BASENAME="${TEST_FILE%.txt}"
    LOG_FILE="$LOG_DIR/${BASENAME}_log.txt"
    
    echo -n "Running $BASENAME... "

    # Execute Valgrind, redirecting output and errors to the log file
    # --log-file is critical for capturing Valgrind's summary
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --log-file="$LOG_FILE" \
             "./$EXEC_NAME" "$TEST_FILE" > /dev/null 2>&1

    # Check the exit status of valgrind (0 is success)
    VALGRIND_EXIT_CODE=$?

    if [ $VALGRIND_EXIT_CODE -eq 0 ]; then
        # Check the Valgrind log for the ultimate summary line
        LEAK_SUMMARY=$(grep "definitely lost" "$LOG_FILE")
        if [[ $LEAK_SUMMARY == *'definitely lost: 0 bytes in 0 blocks'* ]]; then
            echo "SUCCESS (No definite leaks)"
        else
            echo "WARNING (Leaks found. Check $LOG_FILE)"
        fi
    else
        echo "FAIL (Valgrind process error or memory access error)"
    fi

done

echo "--- Test Suite Complete ---"
echo "Check the '$LOG_DIR/' directory for detailed logs."
