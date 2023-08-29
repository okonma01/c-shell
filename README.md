# Simple Linux Shell Implementation

This is a basic implementation of a Linux shell in C. The shell allows users to interact with the underlying operating system by entering commands. It provides a subset of features and functionalities found in typical Unix-like shells.

## Features

- Command execution
- Input/output redirection
- Basic pipeline support
- Built-in commands (e.g., `cd`, `exit`, `help`)

## Getting Started

These instructions will help you compile and run the shell on your local machine.

### Prerequisites

- Linux environment
- GCC compiler

### Compilation

To compile the shell, navigate to the project directory in your terminal and run the following command:

`make`

### Usage

To run the shell, execute the following command:

`./myshell`

You will be presented with a prompt that looks like this:

`>`

You can now enter commands and interact with the shell.

## Examples

### Running a command

To run a command, simply enter the command name followed by any arguments. For example:

`> ls -l`

### Input/output redirection

To redirect the input of a command, use the `<` operator followed by the name of the input file. For example:

`> sort < input.txt`

To redirect the output of a command, use the `>` operator followed by the name of the output file. For example:

`> ls -l > output.txt`

### Pipelines

To pipe the output of one command to the input of another, use the `|` operator. For example:

`> ls -l | sort`

### Built-in commands

The shell supports the following built-in commands:

- `cd` - change directory
- `exit` - exit the shell
- `help` - display help information

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.