#include "../catch.hpp"
#include "../support/test_helpers.hpp"

/*
 * Testes BDD para autenticação de cliente IRC
 * 
 * Baseado no arquivo de feature: tests/bdd/tests/authentication.feature
 * 
 * Cenário: Autenticação bem-sucedida com sequência correta
 */

TEST_CASE("Autenticacao bem-sucedida com sequencia correta", "[authentication]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um cliente conectado com fd 100") {
            Client* client = testServer.addMockClient(100);
            REQUIRE(client != NULL);
            REQUIRE(client->GetFd() == 100);
            
            SECTION("When: o cliente envia PASS secret123") {
                testServer.sendCommand(100, "PASS secret123");
                
                SECTION("Then: a senha deve ser aceita") {
                    REQUIRE(client->isPasswordValid() == true);
                    
                    SECTION("When: o cliente envia NICK john") {
                        testServer.sendCommand(100, "NICK john");
                        
                        SECTION("Then: o nickname deve ser definido como john") {
                            REQUIRE(client->getNickname() == "john");
                            
                            SECTION("When: o cliente envia USER john 0 * :John Doe") {
                                testServer.sendCommand(100, "USER john 0 * :John Doe");
                                
                                SECTION("Then: o cliente deve estar autenticado") {
                                    REQUIRE(client->isAuthenticated() == true);
                                    REQUIRE(client->getUsername() == "john");
                                    REQUIRE(client->getRealname() == "John Doe");
                                    REQUIRE(client->hasCompletedRegistration() == true);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE("Falha na autenticacao com senha incorreta", "[authentication][negative]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um cliente conectado com fd 101") {
            Client* client = testServer.addMockClient(101);
            REQUIRE(client != NULL);
            
            SECTION("When: o cliente envia PASS wrongpassword") {
                testServer.sendCommand(101, "PASS wrongpassword");
                
                SECTION("Then: a senha deve ser rejeitada") {
                    REQUIRE(client->isPasswordValid() == false);
                    
                    SECTION("When: o cliente envia NICK alice e USER alice") {
                        testServer.sendCommand(101, "NICK alice");
                        testServer.sendCommand(101, "USER alice 0 * :Alice Smith");
                        
                        SECTION("Then: o cliente nao deve estar autenticado") {
                            REQUIRE(client->isAuthenticated() == false);
                            REQUIRE(client->hasCompletedRegistration() == false);
                        }
                    }
                }
            }
        }
    }
}

TEST_CASE("Nickname ja em uso", "[authentication][conflict]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um cliente conectado com fd 102") {
            testServer.addMockClient(102);
            
            SECTION("And Given: um cliente conectado com fd 103") {
                testServer.addMockClient(103);
                
                SECTION("When: o cliente 102 envia PASS secret123") {
                    testServer.sendCommand(102, "PASS secret123");
                    
                    SECTION("And When: o cliente 102 envia NICK bob") {
                        testServer.sendCommand(102, "NICK bob");
                        
                        SECTION("And When: o cliente 102 envia USER bob 0 * :Bob Jones") {
                            testServer.sendCommand(102, "USER bob 0 * :Bob Jones");
                            
                            Client* client1 = testServer.getClientByFd(102);
                            REQUIRE(client1 != NULL);
                            REQUIRE(client1->isAuthenticated() == true);
                            REQUIRE(client1->getNickname() == "bob");
                            
                            SECTION("And When: o cliente 103 envia PASS secret123") {
                                testServer.sendCommand(103, "PASS secret123");
                                
                                SECTION("And When: o cliente 103 envia NICK bob") {
                                    testServer.sendCommand(103, "NICK bob");
                                    
                                    SECTION("Then: o cliente 103 deve receber erro de nickname em uso") {
                                        Client* client2 = testServer.getClientByFd(103);
                                        REQUIRE(client2 != NULL);
                                        // O nickname não deve ser definido como "bob"
                                        REQUIRE(client2->getNickname() != "bob");
                                        // O servidor deve ter enviado uma mensagem de erro 433 (ERR_NICKNAMEINUSE)
                                        REQUIRE(MessageCapture::containsMessage(103, "433") == true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

