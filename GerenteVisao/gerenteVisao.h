#ifndef GERENTEVISAO_H
#define GERENTEVISAO_H

#include <messages_robocup_ssl_detection.pb.h>
#include <messages_robocup_ssl_geometry.pb.h>
#include <messages_robocup_ssl_wrapper.pb.h>
#include <mensagem_gerente_visao.pb.h>

#include "definitions.h"
#include "config.h"
#include "bola.h"
#include "robo.h"

class GerenteVisao
{
private:
    vector<RoboSSL >robosSSL, robosAdvSSL;
    vector<char> camerasLidas;


public:
    Bola bola;
    vector<Robo> robosEq; //Robos da Equipe
    vector<Robo> robosAdv; //Robos Adversários

    Config* config;

    int numFrameVisao;

    GerenteVisao(Config* _config);
    ~GerenteVisao();

    void setIdRoboEq(int numRobo, int id);
    void setIdRoboAdv(int numRobo, int id);

    void resetRobosSSL();
    void processPacket(const SSL_DetectionFrame &frameCam);
    void atualizaRobosSSL();

    bool leuTodoCampo();
    void limpaCamerasLidas();

    PacoteVisao montaPacoteVisao();
};

#endif // GERENTEVISAO_H
