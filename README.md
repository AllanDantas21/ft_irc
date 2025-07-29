# FT_IRC

## BOARD
[Team_Board](https://github.com/users/AllanDantas21/projects/3/views/1)

## Link RFC ref
[RFC-2812](https://datatracker.ietf.org/doc/html/rfc2812)

## Links de estudos
[Estudos](./docs/link_utils.md)

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

2. Commands:

    * KICK: Eject a client from the channel
    ```sh
        KICK #channel user
    ```

    * INVITE: Invite a client to a channel
    ```sh
        INVITE user #channel
    ```

    * TOPIC: Change or view the channel topic
    ```sh
        #This command shoud print the TOPIC that has been set to channel
        TOPIC #channel

        #This command should set a new TOPIC to channel
        TOPIC #channel : NEW TOPIC
    ```

    * MODE: Change the channel’s mode, this enable the channel's creator to set some config to channel like an admin.

        - i: Set/remove Invite-only channel
        ```sh
            #This MODE should restrict the users to only access the channel if they have a INVITE
            MODE #channel +i

            #This MODE deactivate the restriction to access the channel with an INVITE
            MODE #channel -i
        ```

        - t: Set/remove the restrictions of the TOPIC command to channel operators
        ```sh
            # This MODE restricts changing the channel TOPIC to only channel operators
            MODE #channel +t

            # This MODE removes the restriction, allowing any user to change the channel TOPIC
            MODE #channel -t
        ```

        - k: Set/remove the channel key (password)
        ```sh
            # This MODE restrict the users to only access the channel if they have known a password
            MODE #channel +k secretkey

            # This MODE removes the restriction, allowing any user to change access the channel
            MODE #channel -k
        ```

        - o: Give/take channel operator privilege
        ```sh
            # This MODE gives privileges to user to be operators
            MODE #channel +o user

            # This MODE removes privileges from users to be operators
            MODE #channel -o user
        ```

        - l: Set/remove the user limit to channel
        ```sh
            # This MODE gives privileges to user to be operators
            MODE #channel +o user

            # This MODE removes privileges from users to be operators
            MODE #channel -o user
        ```



## Project Structure

- `incs/` - Header files
- `srcs/` - Source code files
- `main.cpp` - main source
- `Makefile` - Build script

## Authors

- [Allan Dantas](https://github.com/allandantas21)
- [Thais Franco](https://github.com/thaiismeneses)
- [Daniel Bessa](https://github.com/dwbessa)
