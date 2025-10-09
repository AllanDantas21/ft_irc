#include "../catch.hpp"
#include "../support/test_helpers.hpp"

/*
 * Testes BDD para comandos de canal IRC
 * 
 * Baseado no arquivo de feature: tests/bdd/features/channels.feature
 */

TEST_CASE("Criar e entrar em um novo canal", "[channels][join]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um cliente autenticado") {
            testServer.addMockClient(300);
            testServer.sendCommand(300, "PASS secret123");
            testServer.sendCommand(300, "NICK alice");
            testServer.sendCommand(300, "USER alice 0 * :Alice Smith");
            
            Client* alice = testServer.getClientByFd(300);
            REQUIRE(alice != NULL);
            REQUIRE(alice->isAuthenticated() == true);
            
            SECTION("When: cliente envia JOIN #novochannel") {
                testServer.sendCommand(300, "JOIN #novochannel");
                
                SECTION("Then: cliente deve estar no canal #novochannel") {
                    Channel* channel = testServer.getServer()->FindChannelByName("#novochannel");
                    REQUIRE(channel != NULL);
                    REQUIRE(channel->hasClient(alice) == true);
                    
                    SECTION("And: cliente deve ser operador do canal") {
                        REQUIRE(channel->isOperator(alice) == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Entrar em canal existente", "[channels][join]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um canal existente criado pelo primeiro cliente") {
            testServer.addMockClient(301);
            testServer.sendCommand(301, "PASS secret123");
            testServer.sendCommand(301, "NICK creator");
            testServer.sendCommand(301, "USER creator 0 * :Creator");
            testServer.sendCommand(301, "JOIN #existente");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#existente");
            REQUIRE(channel != NULL);
            
            SECTION("When: um novo cliente autenticado entra no canal") {
                testServer.addMockClient(302);
                testServer.sendCommand(302, "PASS secret123");
                testServer.sendCommand(302, "NICK joiner");
                testServer.sendCommand(302, "USER joiner 0 * :Joiner");
                testServer.sendCommand(302, "JOIN #existente");
                
                Client* joiner = testServer.getClientByFd(302);
                REQUIRE(joiner != NULL);
                
                SECTION("Then: cliente deve estar no canal") {
                    REQUIRE(channel->hasClient(joiner) == true);
                    
                    SECTION("And: não deve ser operador do canal") {
                        REQUIRE(channel->isOperator(joiner) == false);
                    }
                }
            }
        }
    }
}

TEST_CASE("Entrar em canal com senha correta", "[channels][join][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um canal com senha configurada") {
            testServer.addMockClient(303);
            testServer.sendCommand(303, "PASS secret123");
            testServer.sendCommand(303, "NICK owner");
            testServer.sendCommand(303, "USER owner 0 * :Owner");
            testServer.sendCommand(303, "JOIN #secreto");
            testServer.sendCommand(303, "MODE #secreto +k senha123");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#secreto");
            REQUIRE(channel != NULL);
            REQUIRE(channel->hasKey() == true);
            
            SECTION("When: outro cliente entra com senha correta") {
                testServer.addMockClient(304);
                testServer.sendCommand(304, "PASS secret123");
                testServer.sendCommand(304, "NICK guest");
                testServer.sendCommand(304, "USER guest 0 * :Guest");
                testServer.sendCommand(304, "JOIN #secreto senha123");
                
                Client* guest = testServer.getClientByFd(304);
                REQUIRE(guest != NULL);
                
                SECTION("Then: cliente deve estar no canal") {
                    REQUIRE(channel->hasClient(guest) == true);
                }
            }
        }
    }
}

TEST_CASE("Falha ao entrar em canal com senha incorreta", "[channels][join][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um canal com senha configurada") {
            testServer.addMockClient(305);
            testServer.sendCommand(305, "PASS secret123");
            testServer.sendCommand(305, "NICK owner");
            testServer.sendCommand(305, "USER owner 0 * :Owner");
            testServer.sendCommand(305, "JOIN #secreto");
            testServer.sendCommand(305, "MODE #secreto +k senha123");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#secreto");
            REQUIRE(channel != NULL);
            
            SECTION("When: outro cliente tenta entrar com senha errada") {
                testServer.addMockClient(306);
                testServer.sendCommand(306, "PASS secret123");
                testServer.sendCommand(306, "NICK hacker");
                testServer.sendCommand(306, "USER hacker 0 * :Hacker");
                
                MessageCapture::clear(306);
                testServer.sendCommand(306, "JOIN #secreto senhaerrada");
                
                Client* hacker = testServer.getClientByFd(306);
                REQUIRE(hacker != NULL);
                
                SECTION("Then: deve receber erro 475 ERR_BADCHANNELKEY") {
                    bool receivedError = MessageCapture::containsMessage(306, "475");
                    REQUIRE(receivedError == true);
                    REQUIRE(channel->hasClient(hacker) == false);
                }
            }
        }
    }
}

TEST_CASE("Falha ao entrar em canal invite-only sem convite", "[channels][join][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um canal em modo invite-only") {
            testServer.addMockClient(307);
            testServer.sendCommand(307, "PASS secret123");
            testServer.sendCommand(307, "NICK owner");
            testServer.sendCommand(307, "USER owner 0 * :Owner");
            testServer.sendCommand(307, "JOIN #privado");
            testServer.sendCommand(307, "MODE #privado +i");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#privado");
            REQUIRE(channel != NULL);
            REQUIRE(channel->isInviteOnly() == true);
            
            SECTION("When: cliente sem convite tenta entrar") {
                testServer.addMockClient(308);
                testServer.sendCommand(308, "PASS secret123");
                testServer.sendCommand(308, "NICK unwanted");
                testServer.sendCommand(308, "USER unwanted 0 * :Unwanted");
                
                MessageCapture::clear(308);
                testServer.sendCommand(308, "JOIN #privado");
                
                Client* unwanted = testServer.getClientByFd(308);
                REQUIRE(unwanted != NULL);
                
                SECTION("Then: deve receber erro 473 ERR_INVITEONLYCHAN") {
                    bool receivedError = MessageCapture::containsMessage(308, "473");
                    REQUIRE(receivedError == true);
                    REQUIRE(channel->hasClient(unwanted) == false);
                }
            }
        }
    }
}

TEST_CASE("Falha ao entrar em canal cheio", "[channels][join][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um canal com limite de 2 usuarios") {
            testServer.addMockClient(309);
            testServer.sendCommand(309, "PASS secret123");
            testServer.sendCommand(309, "NICK owner");
            testServer.sendCommand(309, "USER owner 0 * :Owner");
            testServer.sendCommand(309, "JOIN #lotado");
            testServer.sendCommand(309, "MODE #lotado +l 2");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#lotado");
            REQUIRE(channel != NULL);
            REQUIRE(channel->getUserLimit() == 2);
            
            SECTION("And Given: segundo usuario entra no canal") {
                testServer.addMockClient(310);
                testServer.sendCommand(310, "PASS secret123");
                testServer.sendCommand(310, "NICK user2");
                testServer.sendCommand(310, "USER user2 0 * :User2");
                testServer.sendCommand(310, "JOIN #lotado");
                
                REQUIRE(channel->getClientCount() == 2);
                
                SECTION("When: terceiro usuario tenta entrar") {
                    testServer.addMockClient(311);
                    testServer.sendCommand(311, "PASS secret123");
                    testServer.sendCommand(311, "NICK user3");
                    testServer.sendCommand(311, "USER user3 0 * :User3");
                    
                    MessageCapture::clear(311);
                    testServer.sendCommand(311, "JOIN #lotado");
                    
                    Client* user3 = testServer.getClientByFd(311);
                    REQUIRE(user3 != NULL);
                    
                    SECTION("Then: deve receber erro 471 ERR_CHANNELISFULL") {
                        bool receivedError = MessageCapture::containsMessage(311, "471");
                        REQUIRE(receivedError == true);
                        REQUIRE(channel->hasClient(user3) == false);
                    }
                }
            }
        }
    }
}

TEST_CASE("Sair de canal com mensagem de despedida", "[channels][part]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: dois clientes no mesmo canal") {
            testServer.addMockClient(313);
            testServer.addMockClient(314);
            
            testServer.sendCommand(313, "PASS secret123");
            testServer.sendCommand(313, "NICK bob");
            testServer.sendCommand(313, "USER bob 0 * :Bob");
            testServer.sendCommand(313, "JOIN #goodbye");
            
            testServer.sendCommand(314, "PASS secret123");
            testServer.sendCommand(314, "NICK charlie");
            testServer.sendCommand(314, "USER charlie 0 * :Charlie");
            testServer.sendCommand(314, "JOIN #goodbye");
            
            Client* bob = testServer.getClientByFd(313);
            Client* charlie = testServer.getClientByFd(314);
            Channel* channel = testServer.getServer()->FindChannelByName("#goodbye");
            
            REQUIRE(channel != NULL);
            REQUIRE(channel->hasClient(bob) == true);
            REQUIRE(channel->hasClient(charlie) == true);
            
            SECTION("When: bob envia PART") {
                MessageCapture::clear(314);
                testServer.sendCommand(313, "PART #goodbye");
                
                SECTION("Then: charlie deve receber mensagem de PART") {
                    bool receivedPart = MessageCapture::containsMessage(314, "PART") &&
                                       MessageCapture::containsMessage(314, "#goodbye");
                    REQUIRE(receivedPart == true);
                }
            }
        }
    }
}

TEST_CASE("Falha ao sair de canal que nao participa", "[channels][part][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: um canal existente e um cliente fora dele") {
            testServer.addMockClient(315);
            testServer.addMockClient(316);
            
            // Criar canal com outro usuario
            testServer.sendCommand(315, "PASS secret123");
            testServer.sendCommand(315, "NICK owner");
            testServer.sendCommand(315, "USER owner 0 * :Owner");
            testServer.sendCommand(315, "JOIN #naoestou");
            
            // Dave nao entra no canal
            testServer.sendCommand(316, "PASS secret123");
            testServer.sendCommand(316, "NICK dave");
            testServer.sendCommand(316, "USER dave 0 * :Dave");
            
            Client* dave = testServer.getClientByFd(316);
            Channel* channel = testServer.getServer()->FindChannelByName("#naoestou");
            REQUIRE(dave != NULL);
            REQUIRE(dave->isAuthenticated() == true);
            REQUIRE(channel != NULL);
            REQUIRE(channel->hasClient(dave) == false);
            
            SECTION("When: tenta sair de canal que nao esta") {
                MessageCapture::clear(316);
                testServer.sendCommand(316, "PART #naoestou");
                
                SECTION("Then: deve receber erro 442 ERR_NOTONCHANNEL") {
                    bool receivedError = MessageCapture::containsMessage(316, "442");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("Operador expulsa usuario com KICK", "[channels][kick]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e membro") {
            testServer.addMockClient(316);
            testServer.addMockClient(317);
            
            testServer.sendCommand(316, "PASS secret123");
            testServer.sendCommand(316, "NICK admin");
            testServer.sendCommand(316, "USER admin 0 * :Admin");
            testServer.sendCommand(316, "JOIN #channel");
            
            testServer.sendCommand(317, "PASS secret123");
            testServer.sendCommand(317, "NICK user");
            testServer.sendCommand(317, "USER user 0 * :User");
            testServer.sendCommand(317, "JOIN #channel");
            
            Client* admin = testServer.getClientByFd(316);
            Client* user = testServer.getClientByFd(317);
            Channel* channel = testServer.getServer()->FindChannelByName("#channel");
            
            REQUIRE(channel != NULL);
            REQUIRE(channel->isOperator(admin) == true);
            REQUIRE(channel->hasClient(user) == true);
            
            SECTION("When: operador expulsa usuario") {
                MessageCapture::clear(317);
                testServer.sendCommand(316, "KICK #channel user :Comportamento inadequado");
                
                SECTION("Then: usuario deve ser removido do canal") {
                    REQUIRE(channel->hasClient(user) == false);
                    
                    SECTION("And: usuario deve receber mensagem de KICK") {
                        bool receivedKick = MessageCapture::containsMessage(317, "KICK");
                        REQUIRE(receivedKick == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Nao-operador tenta KICK", "[channels][kick][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e dois membros") {
            testServer.addMockClient(318);
            testServer.addMockClient(319);
            testServer.addMockClient(320);
            
            testServer.sendCommand(318, "PASS secret123");
            testServer.sendCommand(318, "NICK admin");
            testServer.sendCommand(318, "USER admin 0 * :Admin");
            testServer.sendCommand(318, "JOIN #channel");
            
            testServer.sendCommand(319, "PASS secret123");
            testServer.sendCommand(319, "NICK user1");
            testServer.sendCommand(319, "USER user1 0 * :User1");
            testServer.sendCommand(319, "JOIN #channel");
            
            testServer.sendCommand(320, "PASS secret123");
            testServer.sendCommand(320, "NICK user2");
            testServer.sendCommand(320, "USER user2 0 * :User2");
            testServer.sendCommand(320, "JOIN #channel");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#channel");
            REQUIRE(channel != NULL);
            
            SECTION("When: membro sem privilegios tenta KICK") {
                MessageCapture::clear(319);
                testServer.sendCommand(319, "KICK #channel user2");
                
                SECTION("Then: deve receber erro 482 ERR_CHANOPRIVSNEEDED") {
                    bool receivedError = MessageCapture::containsMessage(319, "482");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("KICK de usuario que nao esta no canal", "[channels][kick][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e usuario fora do canal") {
            testServer.addMockClient(321);
            testServer.addMockClient(322);
            
            testServer.sendCommand(321, "PASS secret123");
            testServer.sendCommand(321, "NICK admin");
            testServer.sendCommand(321, "USER admin 0 * :Admin");
            testServer.sendCommand(321, "JOIN #channel");
            
            testServer.sendCommand(322, "PASS secret123");
            testServer.sendCommand(322, "NICK ghost");
            testServer.sendCommand(322, "USER ghost 0 * :Ghost");
            
            Client* admin = testServer.getClientByFd(321);
            Client* ghost = testServer.getClientByFd(322);
            Channel* channel = testServer.getServer()->FindChannelByName("#channel");
            REQUIRE(channel != NULL);
            REQUIRE(channel->isOperator(admin) == true);
            REQUIRE(channel->hasClient(ghost) == false);
            
            SECTION("When: operador tenta KICK usuario que nao esta no canal") {
                MessageCapture::clear(321);
                testServer.sendCommand(321, "KICK #channel ghost");
                
                SECTION("Then: deve receber erro 441 ERR_USERNOTINCHANNEL") {
                    bool receivedError = MessageCapture::containsMessage(321, "441");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("Convidar usuario para canal com INVITE", "[channels][invite]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal invite-only com operador") {
            testServer.addMockClient(322);
            testServer.addMockClient(323);
            
            testServer.sendCommand(322, "PASS secret123");
            testServer.sendCommand(322, "NICK host");
            testServer.sendCommand(322, "USER host 0 * :Host");
            testServer.sendCommand(322, "JOIN #private");
            testServer.sendCommand(322, "MODE #private +i");
            
            testServer.sendCommand(323, "PASS secret123");
            testServer.sendCommand(323, "NICK guest");
            testServer.sendCommand(323, "USER guest 0 * :Guest");
            
            Client* host = testServer.getClientByFd(322);
            Client* guest = testServer.getClientByFd(323);
            Channel* channel = testServer.getServer()->FindChannelByName("#private");
            
            REQUIRE(channel != NULL);
            REQUIRE(channel->isOperator(host) == true);
            REQUIRE(channel->isInviteOnly() == true);
            
            SECTION("When: operador envia INVITE") {
                MessageCapture::clear(323);
                testServer.sendCommand(322, "INVITE guest #private");
                
                SECTION("Then: convidado deve receber convite") {
                    bool receivedInvite = MessageCapture::containsMessage(323, "INVITE");
                    REQUIRE(receivedInvite == true);
                    
                    SECTION("And: convidado deve conseguir entrar") {
                        testServer.sendCommand(323, "JOIN #private");
                        REQUIRE(channel->hasClient(guest) == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Nao-operador tenta INVITE", "[channels][invite][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e membro") {
            testServer.addMockClient(324);
            testServer.addMockClient(325);
            testServer.addMockClient(326);
            
            testServer.sendCommand(324, "PASS secret123");
            testServer.sendCommand(324, "NICK owner");
            testServer.sendCommand(324, "USER owner 0 * :Owner");
            testServer.sendCommand(324, "JOIN #vip");
            testServer.sendCommand(324, "MODE #vip +i");
            
            testServer.sendCommand(325, "PASS secret123");
            testServer.sendCommand(325, "NICK member");
            testServer.sendCommand(325, "USER member 0 * :Member");
            
            testServer.sendCommand(326, "PASS secret123");
            testServer.sendCommand(326, "NICK friend");
            testServer.sendCommand(326, "USER friend 0 * :Friend");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#vip");
            REQUIRE(channel != NULL);
            
            // Convidar member como owner para ele entrar
            testServer.sendCommand(324, "INVITE member #vip");
            testServer.sendCommand(325, "JOIN #vip");
            
            SECTION("When: membro sem privilegios tenta INVITE") {
                MessageCapture::clear(325);
                testServer.sendCommand(325, "INVITE friend #vip");
                
                SECTION("Then: deve receber erro 482 ERR_CHANOPRIVSNEEDED") {
                    bool receivedError = MessageCapture::containsMessage(325, "482");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("INVITE para usuario ja no canal", "[channels][invite][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e membro") {
            testServer.addMockClient(326);
            testServer.addMockClient(327);
            
            testServer.sendCommand(326, "PASS secret123");
            testServer.sendCommand(326, "NICK op");
            testServer.sendCommand(326, "USER op 0 * :Op");
            testServer.sendCommand(326, "JOIN #room");
            
            testServer.sendCommand(327, "PASS secret123");
            testServer.sendCommand(327, "NICK alice");
            testServer.sendCommand(327, "USER alice 0 * :Alice");
            testServer.sendCommand(327, "JOIN #room");
            
            Client* alice = testServer.getClientByFd(327);
            Channel* channel = testServer.getServer()->FindChannelByName("#room");
            REQUIRE(channel != NULL);
            REQUIRE(channel->hasClient(alice) == true);
            
            SECTION("When: operador tenta INVITE usuario ja no canal") {
                MessageCapture::clear(326);
                testServer.sendCommand(326, "INVITE alice #room");
                
                SECTION("Then: deve receber erro 443 ERR_USERONCHANNEL") {
                    bool receivedError = MessageCapture::containsMessage(326, "443");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("Visualizar topico do canal", "[channels][topic]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com topico definido") {
            testServer.addMockClient(328);
            testServer.sendCommand(328, "PASS secret123");
            testServer.sendCommand(328, "NICK creator");
            testServer.sendCommand(328, "USER creator 0 * :Creator");
            testServer.sendCommand(328, "JOIN #news");
            testServer.sendCommand(328, "TOPIC #news :Bem-vindos ao canal de noticias");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#news");
            REQUIRE(channel != NULL);
            
            SECTION("And Given: outro cliente entra no canal") {
                testServer.addMockClient(329);
                testServer.sendCommand(329, "PASS secret123");
                testServer.sendCommand(329, "NICK reader");
                testServer.sendCommand(329, "USER reader 0 * :Reader");
                testServer.sendCommand(329, "JOIN #news");
                
                SECTION("When: cliente solicita o topico") {
                    MessageCapture::clear(329);
                    testServer.sendCommand(329, "TOPIC #news");
                    
                    SECTION("Then: deve receber o topico") {
                        bool receivedTopic = MessageCapture::containsMessage(329, "332") ||
                                            MessageCapture::containsMessage(329, "Bem-vindos ao canal de noticias");
                        REQUIRE(receivedTopic == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Operador define topico do canal", "[channels][topic]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e membro") {
            testServer.addMockClient(330);
            testServer.addMockClient(331);
            
            testServer.sendCommand(330, "PASS secret123");
            testServer.sendCommand(330, "NICK moderator");
            testServer.sendCommand(330, "USER moderator 0 * :Moderator");
            testServer.sendCommand(330, "JOIN #general");
            
            testServer.sendCommand(331, "PASS secret123");
            testServer.sendCommand(331, "NICK member");
            testServer.sendCommand(331, "USER member 0 * :Member");
            testServer.sendCommand(331, "JOIN #general");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#general");
            REQUIRE(channel != NULL);
            
            SECTION("When: operador define novo topico") {
                MessageCapture::clear(331);
                testServer.sendCommand(330, "TOPIC #general :Novo topico do canal");
                
                SECTION("Then: topico deve ser alterado") {
                    std::string topic = channel->getTopic();
                    REQUIRE(stringContains(topic, "Novo topico do canal") == true);
                    
                    SECTION("And: outros membros devem ser notificados") {
                        bool notified = MessageCapture::containsMessage(331, "TOPIC");
                        REQUIRE(notified == true);
                    }
                }
            }
        }
    }
}

TEST_CASE("Definir topico em canal com modo +t", "[channels][topic][mode][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal em modo +t com operador e membro") {
            testServer.addMockClient(332);
            testServer.addMockClient(333);
            
            testServer.sendCommand(332, "PASS secret123");
            testServer.sendCommand(332, "NICK op");
            testServer.sendCommand(332, "USER op 0 * :Op");
            testServer.sendCommand(332, "JOIN #restricted");
            testServer.sendCommand(332, "MODE #restricted +t");
            
            testServer.sendCommand(333, "PASS secret123");
            testServer.sendCommand(333, "NICK user");
            testServer.sendCommand(333, "USER user 0 * :User");
            testServer.sendCommand(333, "JOIN #restricted");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#restricted");
            REQUIRE(channel != NULL);
            REQUIRE(channel->hasMode('t') == true);
            
            SECTION("When: membro sem privilegios tenta alterar topico") {
                MessageCapture::clear(333);
                testServer.sendCommand(333, "TOPIC #restricted :Tentando mudar");
                
                SECTION("Then: deve receber erro 482 ERR_CHANOPRIVSNEEDED") {
                    bool receivedError = MessageCapture::containsMessage(333, "482");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}

TEST_CASE("Operador ativa modo invite-only", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador") {
            testServer.addMockClient(334);
            testServer.sendCommand(334, "PASS secret123");
            testServer.sendCommand(334, "NICK owner");
            testServer.sendCommand(334, "USER owner 0 * :Owner");
            testServer.sendCommand(334, "JOIN #club");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#club");
            REQUIRE(channel != NULL);
            
            SECTION("When: operador ativa modo +i") {
                testServer.sendCommand(334, "MODE #club +i");
                
                SECTION("Then: canal deve estar em modo invite-only") {
                    REQUIRE(channel->isInviteOnly() == true);
                    REQUIRE(channel->hasMode('i') == true);
                }
            }
        }
    }
}

TEST_CASE("Operador desativa modo invite-only", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal em modo invite-only") {
            testServer.addMockClient(335);
            testServer.sendCommand(335, "PASS secret123");
            testServer.sendCommand(335, "NICK owner");
            testServer.sendCommand(335, "USER owner 0 * :Owner");
            testServer.sendCommand(335, "JOIN #public");
            testServer.sendCommand(335, "MODE #public +i");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#public");
            REQUIRE(channel != NULL);
            REQUIRE(channel->isInviteOnly() == true);
            
            SECTION("When: operador desativa modo -i") {
                testServer.sendCommand(335, "MODE #public -i");
                
                SECTION("Then: canal nao deve estar em modo invite-only") {
                    REQUIRE(channel->isInviteOnly() == false);
                    REQUIRE(channel->hasMode('i') == false);
                }
            }
        }
    }
}

TEST_CASE("Operador ativa modo topic restrito", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador") {
            testServer.addMockClient(336);
            testServer.sendCommand(336, "PASS secret123");
            testServer.sendCommand(336, "NICK owner");
            testServer.sendCommand(336, "USER owner 0 * :Owner");
            testServer.sendCommand(336, "JOIN #discussion");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#discussion");
            REQUIRE(channel != NULL);
            
            SECTION("When: operador ativa modo +t") {
                testServer.sendCommand(336, "MODE #discussion +t");
                
                SECTION("Then: canal deve ter modo +t ativo") {
                    REQUIRE(channel->hasMode('t') == true);
                }
            }
        }
    }
}

TEST_CASE("Operador define senha do canal", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador") {
            testServer.addMockClient(337);
            testServer.sendCommand(337, "PASS secret123");
            testServer.sendCommand(337, "NICK owner");
            testServer.sendCommand(337, "USER owner 0 * :Owner");
            testServer.sendCommand(337, "JOIN #protected");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#protected");
            REQUIRE(channel != NULL);
            
            SECTION("When: operador define senha") {
                testServer.sendCommand(337, "MODE #protected +k secret456");
                
                SECTION("Then: canal deve ter senha configurada") {
                    REQUIRE(channel->hasKey() == true);
                    REQUIRE(channel->checkKey("secret456") == true);
                }
            }
        }
    }
}

TEST_CASE("Operador remove senha do canal", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com senha") {
            testServer.addMockClient(338);
            testServer.sendCommand(338, "PASS secret123");
            testServer.sendCommand(338, "NICK owner");
            testServer.sendCommand(338, "USER owner 0 * :Owner");
            testServer.sendCommand(338, "JOIN #unlocking");
            testServer.sendCommand(338, "MODE #unlocking +k mypass");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#unlocking");
            REQUIRE(channel != NULL);
            REQUIRE(channel->hasKey() == true);
            
            SECTION("When: operador remove senha") {
                testServer.sendCommand(338, "MODE #unlocking -k");
                
                SECTION("Then: canal nao deve ter senha") {
                    REQUIRE(channel->hasKey() == false);
                }
            }
        }
    }
}

TEST_CASE("Operador concede privilegios de operador", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e membro") {
            testServer.addMockClient(339);
            testServer.addMockClient(340);
            
            testServer.sendCommand(339, "PASS secret123");
            testServer.sendCommand(339, "NICK boss");
            testServer.sendCommand(339, "USER boss 0 * :Boss");
            testServer.sendCommand(339, "JOIN #team");
            
            testServer.sendCommand(340, "PASS secret123");
            testServer.sendCommand(340, "NICK assistant");
            testServer.sendCommand(340, "USER assistant 0 * :Assistant");
            testServer.sendCommand(340, "JOIN #team");
            
            Client* assistant = testServer.getClientByFd(340);
            Channel* channel = testServer.getServer()->FindChannelByName("#team");
            REQUIRE(channel != NULL);
            REQUIRE(channel->isOperator(assistant) == false);
            
            SECTION("When: operador concede privilegios") {
                testServer.sendCommand(339, "MODE #team +o assistant");
                
                SECTION("Then: membro deve ser operador") {
                    REQUIRE(channel->isOperator(assistant) == true);
                }
            }
        }
    }
}

TEST_CASE("Operador remove privilegios de operador", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com dois operadores") {
            testServer.addMockClient(341);
            testServer.addMockClient(342);
            
            testServer.sendCommand(341, "PASS secret123");
            testServer.sendCommand(341, "NICK boss");
            testServer.sendCommand(341, "USER boss 0 * :Boss");
            testServer.sendCommand(341, "JOIN #staff");
            
            testServer.sendCommand(342, "PASS secret123");
            testServer.sendCommand(342, "NICK assistant");
            testServer.sendCommand(342, "USER assistant 0 * :Assistant");
            testServer.sendCommand(342, "JOIN #staff");
            
            testServer.sendCommand(341, "MODE #staff +o assistant");
            
            Client* assistant = testServer.getClientByFd(342);
            Channel* channel = testServer.getServer()->FindChannelByName("#staff");
            REQUIRE(channel != NULL);
            REQUIRE(channel->isOperator(assistant) == true);
            
            SECTION("When: operador remove privilegios") {
                testServer.sendCommand(341, "MODE #staff -o assistant");
                
                SECTION("Then: membro nao deve ser mais operador") {
                    REQUIRE(channel->isOperator(assistant) == false);
                }
            }
        }
    }
}

TEST_CASE("Operador define limite de usuarios", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador") {
            testServer.addMockClient(343);
            testServer.sendCommand(343, "PASS secret123");
            testServer.sendCommand(343, "NICK owner");
            testServer.sendCommand(343, "USER owner 0 * :Owner");
            testServer.sendCommand(343, "JOIN #limited");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#limited");
            REQUIRE(channel != NULL);
            
            SECTION("When: operador define limite de 5 usuarios") {
                testServer.sendCommand(343, "MODE #limited +l 5");
                
                SECTION("Then: canal deve ter limite de 5 usuarios") {
                    REQUIRE(channel->getUserLimit() == 5);
                }
            }
        }
    }
}

TEST_CASE("Operador remove limite de usuarios", "[channels][mode]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com limite de usuarios") {
            testServer.addMockClient(344);
            testServer.sendCommand(344, "PASS secret123");
            testServer.sendCommand(344, "NICK owner");
            testServer.sendCommand(344, "USER owner 0 * :Owner");
            testServer.sendCommand(344, "JOIN #unlimited");
            testServer.sendCommand(344, "MODE #unlimited +l 10");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#unlimited");
            REQUIRE(channel != NULL);
            REQUIRE(channel->getUserLimit() == 10);
            
            SECTION("When: operador remove limite") {
                testServer.sendCommand(344, "MODE #unlimited -l");
                
                SECTION("Then: canal nao deve ter limite") {
                    REQUIRE(channel->getUserLimit() == -1);
                }
            }
        }
    }
}

TEST_CASE("Nao-operador tenta alterar modo", "[channels][mode][error]") {
    TestServer testServer;
    
    SECTION("Given: um servidor IRC rodando com senha secret123") {
        testServer.initializeWithPassword("secret123");
        REQUIRE(testServer.getServer() != NULL);
        
        SECTION("And Given: canal com operador e membro") {
            testServer.addMockClient(345);
            testServer.addMockClient(346);
            
            testServer.sendCommand(345, "PASS secret123");
            testServer.sendCommand(345, "NICK op");
            testServer.sendCommand(345, "USER op 0 * :Op");
            testServer.sendCommand(345, "JOIN #nochange");
            
            testServer.sendCommand(346, "PASS secret123");
            testServer.sendCommand(346, "NICK user");
            testServer.sendCommand(346, "USER user 0 * :User");
            testServer.sendCommand(346, "JOIN #nochange");
            
            Channel* channel = testServer.getServer()->FindChannelByName("#nochange");
            REQUIRE(channel != NULL);
            
            SECTION("When: membro sem privilegios tenta alterar modo") {
                MessageCapture::clear(346);
                testServer.sendCommand(346, "MODE #nochange +i");
                
                SECTION("Then: deve receber erro 482 ERR_CHANOPRIVSNEEDED") {
                    bool receivedError = MessageCapture::containsMessage(346, "482");
                    REQUIRE(receivedError == true);
                }
            }
        }
    }
}
