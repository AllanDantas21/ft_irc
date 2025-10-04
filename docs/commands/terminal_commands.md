# Comandos via Terminal

## Comandos Básicos

### KICK: Ejetar cliente do canal
```sh
KICK #channel user
```

### INVITE: Convidar cliente para canal
```sh
INVITE user #channel
```

### TOPIC: Alterar ou visualizar tópico do canal
```sh
# Este comando deve imprimir o TOPIC que foi definido para o canal
TOPIC #channel

# Este comando deve definir um novo TOPIC para o canal
TOPIC #channel : NEW TOPIC
```

### MODE: Alterar modos do canal
Este comando permite ao criador do canal definir algumas configurações como admin.

#### +i/-i: Canal apenas por convite
```sh
# Este MODE deve restringir os usuários a acessar o canal apenas se tiverem um INVITE
MODE #channel +i

# Este MODE desativa a restrição de acesso ao canal com um INVITE
MODE #channel -i
```

#### +t/-t: Restringir comando TOPIC para operadores
```sh
# Este MODE restringe a alteração do TOPIC do canal apenas para operadores
MODE #channel +t

# Este MODE remove a restrição, permitindo que qualquer usuário altere o TOPIC do canal
MODE #channel -t
```

#### +k/-k: Definir/remover chave do canal (senha)
```sh
# Este MODE restringe os usuários a acessar o canal apenas se souberem a senha
MODE #channel +k secretkey

# Este MODE remove a restrição, permitindo que qualquer usuário acesse o canal
MODE #channel -k
```

#### +o/-o: Dar/remover privilégio de operador
```sh
# Este MODE dá privilégios ao usuário para ser operador
MODE #channel +o user

# Este MODE remove privilégios dos usuários para serem operadores
MODE #channel -o user
```

#### +l/-l: Definir/remover limite de usuários
```sh
# Este MODE define um limite de usuários para o canal (ex: máximo 10 usuários)
MODE #channel +l 10

# Este MODE remove o limite de usuários do canal
MODE #channel -l
```

## Comandos Bonus

### DCC SEND: Enviar arquivos diretamente para outro usuário
```sh
# Para enviar um arquivo para outro usuário
# Modo 1:
PRIVMSG user :\001DCC SEND filename\001

# Modo 2:
DCC SEND user filename

# Exemplos:
# Modo 1:
PRIVMSG john :\001DCC SEND document.txt\001

# Modo 2:
DCC SEND john document.txt
```

### DCC GET: Receber arquivos de outro usuário
```sh
# Para aceitar e receber um arquivo que foi enviado para você
DCC GET nickname filename

# Exemplo:
DCC GET mary image.jpg
```
