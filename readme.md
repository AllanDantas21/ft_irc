# FT_IRC

## Link RFC ref
[RFC-2812](https://datatracker.ietf.org/doc/html/rfc2812)

## Links de estudos
[Estudos](./docs/link_utils.md)

## DEVELOPMENT
Veja o estilo de código em:
[codeStyle.md](./docs/codeStyle.md)

## Introduction

ft_irc is a project for the 42 curriculum that involves creating an IRC (Internet Relay Chat) server. The goal of this project is to understand the basics of network programming and the IRC protocol.

## Features

- Handle multiple clients simultaneously
- Support for basic IRC commands (NICK, USER, JOIN, PART, PRIVMSG, etc.)
- Channel management
- User authentication
- Message broadcasting

## Getting Started

### Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/allandantas21/ft_irc.git
    cd ft_irc
    ```

2. Build the project:
    ```sh
    make
    ```

3. Run the server:
    ```sh
    ./ircserv <port> <password>
    ```

## Usage

1. Connect to the server using an IRC client (e.g., nc -> Netcat):
    ```sh
    /nc <localhost> <port>
    ```

## Project Structure

- `incs/` - Header files
- `srcs/` - Source code files
- `main.cpp` - main source
- `Makefile` - Build script

## Authors

- [Allan Dantas](https://github.com/allandantas21)