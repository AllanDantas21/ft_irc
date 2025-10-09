# language: pt
Funcionalidade: Autenticação de Cliente IRC
  Como um cliente IRC
  Quero me autenticar no servidor
  Para poder usar os serviços do IRC

  Cenário: Autenticação bem-sucedida
    Dado um servidor IRC com senha configurada
    Quando um cliente envia credenciais corretas (PASS, NICK, USER)
    Então o cliente deve estar completamente autenticado

  Cenário: Falha na autenticação por senha incorreta
    Dado um servidor IRC com senha configurada
    Quando um cliente envia senha incorreta
    Então o cliente não deve ser autenticado

  Cenário: Conflito de nickname
    Dado um servidor IRC com um cliente autenticado "bob"
    Quando outro cliente tenta usar o mesmo nickname "bob"
    Então deve receber erro de nickname já em uso