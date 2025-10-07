# language: pt
Funcionalidade: Mensagens Privadas IRC
  Como um cliente IRC autenticado
  Quero enviar mensagens privadas
  Para me comunicar com outros usuários e canais

  Cenário: Comunicação direta entre dois usuários
    Dado dois clientes autenticados "alice" e "bob"
    Quando "alice" envia uma mensagem privada para "bob"
    Então "bob" deve receber a mensagem de "alice"

  Cenário: Mensagem para canal com múltiplos usuários
    Dado três clientes autenticados em um canal "#testchannel"
    Quando um cliente envia mensagem para o canal
    Então todos os outros membros do canal devem receber a mensagem

  Cenário: Tentativa de mensagem para usuário inexistente
    Dado um cliente autenticado "frank"
    Quando "frank" tenta enviar mensagem para usuário inexistente
    Então deve receber erro

  Cenário: Mensagem sem autenticação
    Dado um cliente não autenticado
    Quando tenta enviar uma mensagem privada
    Então deve receber erro ERR_NOTREGISTERED

  Cenário: Broadcast em canal
    Dado um canal com três membros autenticados
    Quando um membro envia broadcast para o canal
    Então todos os outros membros devem receber a mensagem

  Cenário: Envio sequencial de mensagens
    Dado dois clientes autenticados "sender" e "receiver"
    Quando "sender" envia três mensagens consecutivas para "receiver"
    Então "receiver" deve receber todas as três mensagens na ordem correta

