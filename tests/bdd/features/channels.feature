# language: pt
Funcionalidade: Comandos de Canal IRC
  Como um cliente IRC autenticado
  Quero gerenciar e interagir com canais
  Para participar de conversas em grupo

  Cenário: Criar e entrar em um novo canal
    Dado um servidor IRC com senha configurada
    Quando um cliente autenticado envia JOIN #novochannel
    Então o cliente deve estar no canal #novochannel
    E o cliente deve ser operador do canal

  Cenário: Entrar em canal existente
    Dado um servidor IRC com um canal existente #existente
    Quando um novo cliente autenticado envia JOIN #existente
    Então o cliente deve estar no canal #existente
    E não deve ser operador do canal

  Cenário: Entrar em canal com senha
    Dado um servidor IRC com um canal #secreto protegido com senha "senha123"
    Quando um cliente autenticado envia JOIN #secreto senha123
    Então o cliente deve estar no canal #secreto

  Cenário: Falha ao entrar em canal com senha incorreta
    Dado um servidor IRC com um canal #secreto protegido com senha "senha123"
    Quando um cliente autenticado envia JOIN #secreto senhaerrada
    Então deve receber erro ERR_BADCHANNELKEY

  Cenário: Falha ao entrar em canal invite-only sem convite
    Dado um servidor IRC com um canal #privado em modo invite-only
    Quando um cliente autenticado sem convite tenta JOIN #privado
    Então deve receber erro ERR_INVITEONLYCHAN

  Cenário: Falha ao entrar em canal cheio
    Dado um servidor IRC com um canal #lotado com limite de 2 usuários
    E o canal #lotado já tem 2 membros
    Quando um terceiro cliente tenta JOIN #lotado
    Então deve receber erro ERR_CHANNELISFULL

  Cenário: Sair de um canal com PART
    Dado um cliente autenticado "alice" em um canal #test
    Quando "alice" envia PART #test
    Então "alice" não deve mais estar no canal #test

  Cenário: Sair de canal com mensagem de despedida
    Dado um cliente autenticado "bob" em um canal #goodbye
    E outro cliente "charlie" também está no canal #goodbye
    Quando "bob" envia PART #goodbye :Ate logo pessoal
    Então "charlie" deve receber mensagem de PART com "Ate logo pessoal"

  Cenário: Falha ao sair de canal que não participa
    Dado um cliente autenticado "dave"
    Quando "dave" tenta PART #naoestou
    Então deve receber erro ERR_NOTONCHANNEL

  Cenário: Operador expulsa usuário com KICK
    Dado um canal #channel com operador "admin" e membro "user"
    Quando "admin" envia KICK #channel user :Comportamento inadequado
    Então "user" deve ser removido do canal
    E "user" deve receber mensagem de KICK

  Cenário: Não-operador tenta KICK
    Dado um canal #channel com operador "admin" e membro "user1" e "user2"
    Quando "user1" tenta KICK #channel user2
    Então deve receber erro ERR_CHANOPRIVSNEEDED

  Cenário: KICK de usuário que não está no canal
    Dado um canal #channel com operador "admin"
    Quando "admin" tenta KICK #channel ghost
    Então deve receber erro ERR_USERNOTINCHANNEL

  Cenário: Convidar usuário para canal com INVITE
    Dado um canal #private em modo invite-only com operador "host"
    E um cliente autenticado "guest" não está no canal
    Quando "host" envia INVITE guest #private
    Então "guest" deve receber convite para #private
    E "guest" deve conseguir entrar no canal

  Cenário: Não-operador tenta INVITE
    Dado um canal #vip com operador "owner" e membro "member"
    Quando "member" tenta INVITE friend #vip
    Então deve receber erro ERR_CHANOPRIVSNEEDED

  Cenário: INVITE para usuário já no canal
    Dado um canal #room com operador "op" e membro "alice"
    Quando "op" tenta INVITE alice #room
    Então deve receber erro ERR_USERONCHANNEL

  Cenário: Visualizar tópico do canal
    Dado um canal #news com tópico "Bem-vindos ao canal de noticias"
    E um cliente autenticado "reader" no canal
    Quando "reader" envia TOPIC #news
    Então deve receber o tópico "Bem-vindos ao canal de noticias"

  Cenário: Operador define tópico do canal
    Dado um canal #general com operador "moderator"
    Quando "moderator" envia TOPIC #general :Novo topico do canal
    Então o tópico do canal deve ser "Novo topico do canal"
    E outros membros devem ser notificados da mudança

  Cenário: Definir tópico em canal com modo +t
    Dado um canal #restricted em modo +t com operador "op" e membro "user"
    Quando "user" tenta TOPIC #restricted :Tentando mudar
    Então deve receber erro ERR_CHANOPRIVSNEEDED

  Cenário: Operador ativa modo invite-only
    Dado um canal #club com operador "owner"
    Quando "owner" envia MODE #club +i
    Então o canal deve estar em modo invite-only

  Cenário: Operador desativa modo invite-only
    Dado um canal #public em modo invite-only com operador "owner"
    Quando "owner" envia MODE #public -i
    Então o canal não deve estar em modo invite-only

  Cenário: Operador ativa modo topic restrito
    Dado um canal #discussion com operador "owner"
    Quando "owner" envia MODE #discussion +t
    Então o canal deve ter modo +t ativo

  Cenário: Operador define senha do canal
    Dado um canal #protected com operador "owner"
    Quando "owner" envia MODE #protected +k secret456
    Então o canal deve ter senha configurada

  Cenário: Operador remove senha do canal
    Dado um canal #unlocking com senha e operador "owner"
    Quando "owner" envia MODE #unlocking -k
    Então o canal não deve ter senha

  Cenário: Operador concede privilégios de operador
    Dado um canal #team com operador "boss" e membro "assistant"
    Quando "boss" envia MODE #team +o assistant
    Então "assistant" deve ser operador do canal

  Cenário: Operador remove privilégios de operador
    Dado um canal #staff com operadores "boss" e "assistant"
    Quando "boss" envia MODE #staff -o assistant
    Então "assistant" não deve ser operador do canal

  Cenário: Operador define limite de usuários
    Dado um canal #limited com operador "owner"
    Quando "owner" envia MODE #limited +l 5
    Então o canal deve ter limite de 5 usuários

  Cenário: Operador remove limite de usuários
    Dado um canal #unlimited com limite e operador "owner"
    Quando "owner" envia MODE #unlimited -l
    Então o canal não deve ter limite de usuários

  Cenário: Não-operador tenta alterar modo
    Dado um canal #nochange com operador "op" e membro "user"
    Quando "user" tenta MODE #nochange +i
    Então deve receber erro ERR_CHANOPRIVSNEEDED
