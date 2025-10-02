#!/usr/bin/bash
# Created on: Thu Sep 18 15:43:41 +01 202

# --- Valgrind Automation Script for wish Shell ---

# 1. Configuration
EXEC_NAME="wish"
TEST_DIR="batch_tests"
LOG_DIR="valgrind_logs"

if [ -z $1 ]; then
	echo "Usage: $0 <test_index>" 2>&1
	exit 1
fi
# Ensure the executable exists
if [ ! -x "./$EXEC_NAME" ]; then
    echo "ERROR: Executable './$EXEC_NAME' not found."
fi


# Create logging directory
mkdir -p $LOG_DIR
mkdir -p $TEST_DIR

# List of all test files to run
TEST_FILES=(
    "$TEST_DIR/test_01_builtins_path_misuse.txt"
    "$TEST_DIR/test_02_redirection_parsing.txt"
    "$TEST_DIR/test_03_parallel_mixed_errors.txt"
    "$TEST_DIR/test_04_success_and_termination.txt"
)
TEST_FILE=${TEST_FILES[${1}]}
LOG_FILE=${TEST_FILE##*/}
LOG_FILE=${LOG_FILE%.txt}
LOG_FILE=$LOG_DIR/$LOG_FILE

# Execute Valgrind, redirecting output and errors to the log file
# --log-file is critical for capturing Valgrind's summary

valgrind --leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--log-file="$LOG_FILE" \
	"./$EXEC_NAME" "$TEST_FILE"

cat $LOG_FILE

