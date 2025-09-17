#!/bin/bash
# =============================================================================
# dbmanager Automated Test and Valgrind Script
# =============================================================================

# Define program and file names
DB_PROGRAM="kv-v1.out"
DB_SOURCE="kv-v1.c"
DB_FILE="database.txt"
VALGRIND_LOG="valgrind-log.txt"
WORDLIST="/usr/share/dict/words"

# --- Utility Functions ---

# == Validation and Prerequisites  ============================================
# Function to validate the count argument.
_validate_count() {
    local count="$1"

    if [[ -z "$count" ]]; then
        command echo "Error: Missing argument for count." >&2
        return 1
    fi

    if ! [[ "$count" =~ ^[0-9]+$ ]]; then
        command echo "Error: The argument '$count' is not a valid number." >&2
        return 1
    fi

    if (( count <= 0 )); then
        command echo "Error: The count must be a positive number greater than 0." >&2
        return 1
    fi

    return 0
}

_usage() {
	command cat <<-EOF

	Usage: ./${0##*/} {seed|val|test|clean}

	seed <count>: Populate the database with random entries.
	val <count>:  Runs a memory leak check and saves output to a log file.
	test:         Run some tests.
	clean:        Removes compiled executable and generated files.

	EOF
}

# Function to ensure the C program is compiled.
_compile_program() {
    if [ ! -f "$DB_PROGRAM" ]; then
        command echo "Executable '$DB_PROGRAM' not found. Attempting to compile..."
        command make "$DB_PROGRAM"
        if [ $? -ne 0 ]; then
            command echo "Error: make failed to compile. Trying with gcc..."
            command gcc -o "$DB_PROGRAM" "$DB_SOURCE"
            if [ $? -ne 0 ]; then
                command echo "Error: Compilation failed. Please fix errors in '$DB_SOURCE'."
                exit 1
            fi
        fi
    fi
}
# =============================================================================

# Function to generate a single random word.
_generate_word() {
    local value=""
    if [ ! -f "$WORDLIST" ]; then
        command echo "Error: Word list not found at '$WORDLIST'." >&2
        command echo "Please install a wordlist package (e.g., 'wordlist' or 'words' on your system)." >&2
        exit 1
    fi
    # Use 'shuf' to select one random line from the word list.
    value=$(shuf -n 1 "$WORDLIST")
    command echo ${value%\'s}
}

# Function to populate the database with a specified number of arbitrary key-value pairs.
_populate_db() {
    local count="$1"
    command echo "Populating database with $count arbitrary entries..."

    # Clean any existing database file
    command rm -f "$DB_FILE"

    local key_list=($(shuf -i 10-99 -n ${count}))

    local value_list=()
    local i=0
    until ((i++ == count));do
        value_list+=($(_generate_word))
    done

    local cmd_str=""
    for i in $(seq 0 $((count-1))); do
        cmd_str+=" p,${key_list[$i]},${value_list[$i]}"
    done

    # Execute the command to populate the database.
    # We use eval to handle the dynamic command string.
    command eval ./"$DB_PROGRAM" "$cmd_str" > /dev/null 2>&1
}

# ==  Test helper functions  ===================================================
# Function to print a stylized header for each test section.
_print_header() {
	command cat <<-EOF

		=================================================
		  "${1}"
		=================================================
	EOF
}

# Function to assert that two files have the same content.
_assert_files_equal() {
  local test_name="$1"
  local actual_file="$2"
  local expected_file="$3"

  if diff -u "$actual_file" "$expected_file" >/dev/null 2>&1; then
    echo "✅ Passed: $test_name"
  else
    echo "❌ Failed: $test_name"
    echo "--- Expected Output ---"
    cat "$expected_file"
    echo "--- Actual Output ---"
    cat "$actual_file"
    exit 1
  fi
}

# Function to run all the tests.
_run_tests() {
	_compile_program

	# Clean up any existing database files from previous runs.
	rm -f "$DB_FILE"

	# ==  TEST 1  ==================================================================

	_print_header "Test 1: Initial Population (p)"
	# Populate the database with 5 key-value pairs.
	./"$DB_PROGRAM" p,1,one p,2,two p,3,three p,4,four p,5,five > /dev/null

	# Verify the populated database content using 'a'.
	./"$DB_PROGRAM" a > initial_output.txt

	# Expected content for the initial database.
	cat <<-EOF > expected_initial_output.txt
	1,one
	2,two
	3,three
	4,four
	5,five
	EOF
	_assert_files_equal "Verify initial population" initial_output.txt expected_initial_output.txt

	# Clean up temporary files.
	rm -f initial_output.txt expected_initial_output.txt

	# ==  TEST 2  ==================================================================

	_print_header "Test 2: Single Key Retrieval (g)"
	# Get the value for key 3.
	./"$DB_PROGRAM" g,3 > get_output.txt 2>&1
	echo "3,three" > expected_get_output.txt
	_assert_files_equal "Verify key '3' is retrieved correctly" get_output.txt expected_get_output.txt
	rm -f get_output.txt expected_get_output.txt

	# ==  TEST 3  ==================================================================

	_print_header "Test 3: Non-existent Key Retrieval (g)"
	# Try to get a non-existent key.
	./"$DB_PROGRAM" g,99 > get_output_not_found.txt 2>&1
	echo "99 not found" > expected_get_not_found.txt
	_assert_files_equal "Verify key '99' is not found" get_output_not_found.txt expected_get_not_found.txt
	rm -f get_output_not_found.txt expected_get_not_found.txt

	# ==  TEST 4  ==================================================================

	_print_header "Test 4: Update an Existing Key (p)"
	# Update the value of key 3.
	./"$DB_PROGRAM" p,3,THREE-updated > /dev/null

	# Verify the updated database content.
	./"$DB_PROGRAM" a > update_output.txt
	cat <<-EOF > expected_update_output.txt
	1,one
	2,two
	3,THREE-updated
	4,four
	5,five
	EOF
	_assert_files_equal "Verify key '3' is updated" update_output.txt expected_update_output.txt
	rm -f update_output.txt expected_update_output.txt

	# ==  TEST 5  ==================================================================

	_print_header "Test 5: Delete a Key (d)"
	# Delete key 3.
	./"$DB_PROGRAM" d,3 > /dev/null

	# Verify that key 3 is no longer in the database.
	./"$DB_PROGRAM" a > delete_output.txt
	cat <<-EOF > expected_delete_output.txt
	1,one
	2,two
	4,four
	5,five
	EOF
	_assert_files_equal "Verify key '3' is deleted" delete_output.txt expected_delete_output.txt
	rm -f delete_output.txt expected_delete_output.txt

	# ==  TEST 6  ==================================================================

	_print_header "Test 6: Clear the Database (c)"
	# Clear the entire database.
	./"$DB_PROGRAM" c > /dev/null

	# Verify that the database file has been unlinked (deleted).
	if [ -e "$DB_FILE" ]; then
	  echo "❌ Failed: Database file still exists after clear."
	  exit 1
	else
	  echo "✅ Passed: Database file was unlinked."
	fi

	# ==  TEST 7  ==================================================================

	_print_header "Test 7: Verify 'a' on an empty database"
	# Verify 'a' on an empty database.
	./"$DB_PROGRAM" a > clear_output.txt 2>&1
	echo "Database is empty." > expected_clear_output.txt
	_assert_files_equal "Verify empty database was notified" clear_output.txt expected_clear_output.txt
	rm -f clear_output.txt expected_clear_output.txt

	# ==  TEST 8  ==================================================================

	_print_header "Test 8: Combined Operations"
	# Perform a series of operations in a single run.
	./"$DB_PROGRAM" p,10,alpha p,4,FOUR-new 2>&1
	./"$DB_PROGRAM" d,2 g,10 g,4 > combined_output.txt 2>&1
	cat <<-EOF > expected_combined_output.txt
	Could not delete entry 2: No such entry.
	10,alpha
	4,FOUR-new
	EOF
	_assert_files_equal "Verify combined operations" combined_output.txt expected_combined_output.txt
	rm -f combined_output.txt expected_combined_output.txt

	# ==  Final Clean up  ==========================================================
	rm -f "$DB_FILE"
	echo ""
	echo "All tests passed successfully!"
}

# =============================================================================
# MAIN ENTRY POINT
# =============================================================================

case "$1" in
    "s"|"seed")
        _validate_count "$2" || exit 1
        _compile_program
        echo "--- Populate database ---"
        _populate_db "$2"
        echo "Display database..."
        ./"$DB_PROGRAM" a
        ;;

    "v"|"val")
        _validate_count "$2" || exit 1
        _compile_program
        echo "--- Running test with Valgrind ---"
        echo "Output will be saved to '$VALGRIND_LOG'."

        # We run the population first in a standard way.
        _populate_db "$2"

        # Then we run a few operations under Valgrind's watchful eye.
        valgrind --leak-check=full \
            --show-leak-kinds=all \
            --log-file="$VALGRIND_LOG" \
            ./"$DB_PROGRAM" g,23 d,46 p,79,new_value a

        echo ""
        echo "Valgrind finished. Review the output in '$VALGRIND_LOG'."
        ;;

    "t"|"test")
		_run_tests
        ;;
    "c"|"clean")
        echo "Cleaning up generated files..."
        rm -f "$DB_PROGRAM" "$DB_FILE" "$VALGRIND_LOG"
        ;;

    *)
		_usage
        exit 1
        ;;
esac
