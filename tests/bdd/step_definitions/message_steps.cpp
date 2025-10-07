#include "../catch.hpp"
#include "../support/test_helpers.hpp"
#include <iostream>

/*
 * Testes BDD para mensagens privadas IRC (PRIVMSG)
 * 
 * Baseado no arquivo de feature: tests/bdd/tests/messages.feature
 */

TEST_CASE("Enviar mensagem privada entre dois usuarios autenticados", "[messages][privmsg]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: dois clientes conectados") {
            testServer.addMockClient(200);
            testServer.addMockClient(201);
            
            SECTION("When: ambos os clientes se autenticam") {
                // Autenticar Alice
                testServer.sendCommand(200, "PASS secret123");
                testServer.sendCommand(200, "NICK alice");
                testServer.sendCommand(200, "USER alice 0 * :Alice Smith");
                
                Client* alice = testServer.getClientByFd(200);
                REQUIRE(alice != NULL);
                REQUIRE(alice->isAuthenticated() == true);
                REQUIRE(alice->getNickname() == "alice");
                
                // Autenticar Bob
                testServer.sendCommand(201, "PASS secret123");
                testServer.sendCommand(201, "NICK bob");
                testServer.sendCommand(201, "USER bob 0 * :Bob Jones");
                
                Client* bob = testServer.getClientByFd(201);
                REQUIRE(bob != NULL);
                REQUIRE(bob->isAuthenticated() == true);
                REQUIRE(bob->getNickname() == "bob");
                
                SECTION("When: Alice envia mensagem privada para Bob") {
                    MessageCapture::clear(201);
                    testServer.sendCommand(200, "PRIVMSG bob :Hello Bob!");
                    
                    SECTION("Then: Bob deve receber a mensagem de Alice") {
                        std::vector<std::string> bobMessages = MessageCapture::getMessages(201);
                        bool receivedMessage = false;
                        
                        for (size_t i = 0; i < bobMessages.size(); i++) {
                            if (stringContains(bobMessages[i], "alice") && 
                                stringContains(bobMessages[i], "PRIVMSG") &&
                                stringContains(bobMessages[i], "Hello Bob!")) {
                                receivedMessage = true;
                                break;
                            }
                        }
                        
                        REQUIRE(receivedMessage == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Enviar mensagem para canal com multiplos usuarios", "[messages][channel]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: tres clientes conectados e autenticados") {
            testServer.addMockClient(202);
            testServer.addMockClient(203);
            testServer.addMockClient(204);
            
            // Autenticar Charlie
            testServer.sendCommand(202, "PASS secret123");
            testServer.sendCommand(202, "NICK charlie");
            testServer.sendCommand(202, "USER charlie 0 * :Charlie Brown");
            
            Client* charlie = testServer.getClientByFd(202);
            REQUIRE(charlie != NULL);
            REQUIRE(charlie->isAuthenticated() == true);
            
            // Autenticar Dave
            testServer.sendCommand(203, "PASS secret123");
            testServer.sendCommand(203, "NICK dave");
            testServer.sendCommand(203, "USER dave 0 * :Dave Wilson");
            
            Client* dave = testServer.getClientByFd(203);
            REQUIRE(dave != NULL);
            REQUIRE(dave->isAuthenticated() == true);
            
            // Autenticar Eve
            testServer.sendCommand(204, "PASS secret123");
            testServer.sendCommand(204, "NICK eve");
            testServer.sendCommand(204, "USER eve 0 * :Eve Davis");
            
            Client* eve = testServer.getClientByFd(204);
            REQUIRE(eve != NULL);
            REQUIRE(eve->isAuthenticated() == true);
            
            SECTION("And Given: todos entram no mesmo canal") {
                testServer.sendCommand(202, "JOIN #testchannel");
                testServer.sendCommand(203, "JOIN #testchannel");
                testServer.sendCommand(204, "JOIN #testchannel");
                
                SECTION("When: Charlie envia mensagem para o canal") {
                    MessageCapture::clear(203);
                    MessageCapture::clear(204);
                    testServer.sendCommand(202, "PRIVMSG #testchannel :Hello everyone!");
                    
                    SECTION("Then: Dave e Eve devem receber a mensagem") {
                        bool daveReceived = MessageCapture::containsMessage(203, "testchannel") &&
                                           MessageCapture::containsMessage(203, "Hello everyone!");
                        bool eveReceived = MessageCapture::containsMessage(204, "testchannel") &&
                                          MessageCapture::containsMessage(204, "Hello everyone!");
                        
                        REQUIRE(daveReceived == true);
                        REQUIRE(eveReceived == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Enviar mensagem para usuario inexistente", "[messages][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um cliente conectado e autenticado") {
            Client* frank = testServer.addMockClient(205);
            
            testServer.sendCommand(205, "PASS secret123");
            testServer.sendCommand(205, "NICK frank");
            testServer.sendCommand(205, "USER frank 0 * :Frank Miller");
            REQUIRE(frank->isAuthenticated() == true);
            
            SECTION("When: envia mensagem para usuário inexistente") {
                MessageCapture::clear(205);
                testServer.sendCommand(205, "PRIVMSG ghost :Do you exist?");
                
                SECTION("Then: deve receber erro 401 ERR_NOSUCHNICK") {
                    bool receivedError = MessageCapture::containsMessage(205, "401");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("Enviar mensagem sem estar autenticado", "[messages][error][authentication]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um cliente conectado mas não autenticado") {
            Client* client = testServer.addMockClient(206);
            REQUIRE(client != NULL);
            REQUIRE(client->isAuthenticated() == false);
            
            SECTION("When: tenta enviar mensagem") {
                MessageCapture::clear(206);
                testServer.sendCommand(206, "PRIVMSG someone :Hello!");
                
                SECTION("Then: deve receber erro 451 ERR_NOTREGISTERED") {
                    bool receivedError = MessageCapture::containsMessage(206, "451");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("Broadcast de mensagens para todos em um canal", "[messages][channel][broadcast]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: tres clientes conectados e autenticados") {
            testServer.addMockClient(207);
            testServer.addMockClient(208);
            testServer.addMockClient(209);
            
            // Autenticar Sender
            testServer.sendCommand(207, "PASS secret123");
            testServer.sendCommand(207, "NICK sender");
            testServer.sendCommand(207, "USER sender 0 * :Message Sender");
            
            Client* sender = testServer.getClientByFd(207);
            REQUIRE(sender != NULL);
            REQUIRE(sender->isAuthenticated() == true);
            
            // Autenticar Receiver1
            testServer.sendCommand(208, "PASS secret123");
            testServer.sendCommand(208, "NICK receiver1");
            testServer.sendCommand(208, "USER receiver1 0 * :First Receiver");
            
            Client* receiver1 = testServer.getClientByFd(208);
            REQUIRE(receiver1 != NULL);
            REQUIRE(receiver1->isAuthenticated() == true);
            
            // Autenticar Receiver2
            testServer.sendCommand(209, "PASS secret123");
            testServer.sendCommand(209, "NICK receiver2");
            testServer.sendCommand(209, "USER receiver2 0 * :Second Receiver");
            
            Client* receiver2 = testServer.getClientByFd(209);
            REQUIRE(receiver2 != NULL);
            REQUIRE(receiver2->isAuthenticated() == true);
            
            SECTION("And Given: todos entram no canal #broadcast") {
                testServer.sendCommand(207, "JOIN #broadcast");
                testServer.sendCommand(208, "JOIN #broadcast");
                testServer.sendCommand(209, "JOIN #broadcast");
                
                SECTION("When: sender envia broadcast para o canal") {
                    MessageCapture::clear(208);
                    MessageCapture::clear(209);
                    testServer.sendCommand(207, "PRIVMSG #broadcast :Message for everyone");
                    
                    SECTION("Then: ambos os receivers devem receber a mensagem") {
                        bool receiver1Got = MessageCapture::containsMessage(208, "Message for everyone");
                        bool receiver2Got = MessageCapture::containsMessage(209, "Message for everyone");
                        
                        REQUIRE(receiver1Got == true);
                        REQUIRE(receiver2Got == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Multiplas mensagens em sequencia", "[messages][sequence]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: dois clientes conectados e autenticados") {
            testServer.addMockClient(210);
            testServer.addMockClient(211);
            
            // Autenticar Sender2
            testServer.sendCommand(210, "PASS secret123");
            testServer.sendCommand(210, "NICK sender2");
            testServer.sendCommand(210, "USER sender2 0 * :Second Sender");
            
            Client* sender2 = testServer.getClientByFd(210);
            REQUIRE(sender2 != NULL);
            REQUIRE(sender2->isAuthenticated() == true);
            
            // Autenticar Receiver3
            testServer.sendCommand(211, "PASS secret123");
            testServer.sendCommand(211, "NICK receiver3");
            testServer.sendCommand(211, "USER receiver3 0 * :Third Receiver");
            
            Client* receiver3 = testServer.getClientByFd(211);
            REQUIRE(receiver3 != NULL);
            REQUIRE(receiver3->isAuthenticated() == true);
            
            SECTION("When: sender2 envia multiplas mensagens em sequencia") {
                MessageCapture::clear(211);
                testServer.sendCommand(210, "PRIVMSG receiver3 :First message");
                testServer.sendCommand(210, "PRIVMSG receiver3 :Second message");
                testServer.sendCommand(210, "PRIVMSG receiver3 :Third message");
                
                SECTION("Then: receiver3 deve ter recebido 3 mensagens de sender2") {
                    std::vector<std::string> messages = MessageCapture::getMessages(211);
                    int messageCount = 0;
                    
                    for (size_t i = 0; i < messages.size(); i++) {
                        if (stringContains(messages[i], "sender2") && 
                            stringContains(messages[i], "PRIVMSG")) {
                            messageCount++;
                        }
                    }
                    
                    REQUIRE(messageCount == 3);
                    
                    // Verificar conteúdo das mensagens
                    bool hasFirst = MessageCapture::containsMessage(211, "First message");
                    bool hasSecond = MessageCapture::containsMessage(211, "Second message");
                    bool hasThird = MessageCapture::containsMessage(211, "Third message");
                    
                    REQUIRE(hasFirst == true);
                    REQUIRE(hasSecond == true);
                    REQUIRE(hasThird == true);
                }
            }
        }
    }
}

