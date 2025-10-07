# language: pt
Funcionalidade: Autenticação de Cliente IRC
  Como um cliente IRC
  Quero me autenticar no servidor
  Para poder usar os serviços do IRC

  Cenário: Autenticação bem-sucedida com sequência correta
    Dado um servidor IRC rodando com senha "secret123"
    E um cliente conectado com fd 100
    Quando o cliente envia "PASS secret123"
    Então a senha deve ser aceita
    Quando o cliente envia "NICK john"
    Então o nickname deve ser definido como "john"
    Quando o cliente envia "USER john 0 * :John Doe"
    Então o cliente deve estar autenticado
    E o username deve ser "john"

  Cenário: Falha na autenticação com senha incorreta
    Dado um servidor IRC rodando com senha "secret123"
    E um cliente conectado com fd 101
    Quando o cliente envia "PASS wrongpassword"
    Então a senha deve ser rejeitada
    Quando o cliente envia "NICK alice"
    E o cliente envia "USER alice 0 * :Alice Smith"
    Então o cliente não deve estar autenticado

  Cenário: Nickname já em uso
    Dado um servidor IRC rodando com senha "secret123"
    E um cliente conectado com fd 102
    E um cliente conectado com fd 103
    Quando o cliente 102 envia "PASS secret123"
    E o cliente 102 envia "NICK bob"
    E o cliente 102 envia "USER bob 0 * :Bob Jones"
    E o cliente 103 envia "PASS secret123"
    E o cliente 103 envia "NICK bob"
    Então o cliente 103 deve receber erro de nickname em uso

