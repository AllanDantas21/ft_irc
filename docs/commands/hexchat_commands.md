# Comandos IRC no HexChat

Este documento descreve como executar os comandos IRC do servidor ft_irc usando o cliente HexChat, que possui algumas diferenças na sintaxe comparado ao uso direto via terminal.

## Conectando ao Servidor

1. Abra o HexChat
2. Vá em **Server List** (ou pressione `Ctrl+S`)
3. Clique em **Add** para adicionar um novo servidor
4. Configure:
   - **Network**: Nome personalizado (ex: "FT_IRC")
   - **Server**: `localhost`
   - **Port**: Porta do servidor (ex: `6667`)
   - **Password**: Senha do servidor
5. Clique em **Connect**

## Comandos Básicos

### KICK - Ejetar um cliente do canal
```
/kick #canal usuario
```
**Exemplo:**
```
/kick #general joao
```

### INVITE - Convidar um cliente para um canal
```
/invite usuario #canal
```
**Exemplo:**
```
/invite maria #general
```

### TOPIC - Alterar ou visualizar o tópico do canal

**Visualizar o tópico atual:**
```
/topic #canal
```

**Definir um novo tópico:**
```
/topic #canal Novo tópico do canal
```
**Exemplo:**
```
/topic #general Bem-vindos ao canal geral!
```

### MODE - Alterar o modo do canal

#### i - Definir/remover canal apenas por convite
```
/mode #canal +i
/mode #canal -i
```

#### t - Definir/remover restrições do comando TOPIC para operadores
```
/mode #canal +t
/mode #canal -t
```

#### k - Definir/remover chave do canal (senha)
```
/mode #canal +k senhasecreta
/mode #canal -k
```

#### o - Dar/remover privilégios de operador do canal
```
/mode #canal +o usuario
/mode #canal -o usuario
```

#### l - Definir/remover limite de usuários do canal
```
/mode #canal +l 10
/mode #canal -l
```

## Comandos Bônus

### DCC SEND - Enviar arquivos diretamente para outro usuário

**Modo 1 (via PRIVMSG):**
```
/msg usuario :\001DCC SEND nomearquivo\001
```

**Modo 2 (comando direto):**
```
/dcc send usuario nomearquivo
```

**Exemplos:**
```
/msg joao :\001DCC SEND documento.txt\001
/dcc send joao documento.txt
```

### DCC GET - Receber arquivos de outro usuário
```
/dcc get apelido nomearquivo
```

**Exemplo:**
```
/dcc get maria imagem.jpg
```

## Diferenças Importantes

### Interface Gráfica
- No HexChat, você pode usar tanto a barra de comando quanto clicar com o botão direito nos usuários/canais para acessar opções
- O HexChat mostra notificações visuais para mensagens privadas e menções

### Navegação
- Use `Ctrl+Tab` para alternar entre canais
- Use `Ctrl+Page Up/Down` para navegar entre servidores
- Clique duplo em um usuário para abrir uma conversa privada

### Configurações
- Acesse as configurações via **HexChat > Preferences** (ou `Ctrl+Shift+P`)
- Configure cores, fontes e comportamento do cliente

## Dicas Úteis

1. **Auto-complete**: Use `Tab` para completar automaticamente nomes de usuários e canais
2. **Histórico**: Use as setas `↑/↓` para navegar pelo histórico de comandos
3. **Canais**: Clique com o botão direito em um canal para ver opções como "Join", "Part", etc.
4. **Usuários**: Clique com o botão direito em um usuário para ver opções como "Private Message", "Kick", "Ban", etc.

## Troubleshooting

- Se não conseguir conectar, verifique se o servidor está rodando e se a porta está correta
- Para desconectar, use `/quit` ou feche o HexChat
- Para reconectar, use `/reconnect` ou `Ctrl+R`

