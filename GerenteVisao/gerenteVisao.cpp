#include <gerenteVisao.h>

void GerenteVisao::limpaCamerasLidas()
{
    for(int i; i<NUM_CAMERAS; i++){
       camerasLidas[i] = false;
    }
    /*foreach (char camera, camerasLidas)
    {
        camera = false;
    }*/
}

GerenteVisao::GerenteVisao(Config* _config):
    camerasLidas(NUM_CAMERAS),
    robosAdvSSL(NUM_MAX_ROBOS),
    robosSSL(NUM_MAX_ROBOS)
{
    limpaCamerasLidas();
    resetRobosSSL();

    numFrameVisao=0;
    config = _config;

    for(int id=0;id<NUM_MAX_ROBOS;id++){
        robosEq.push_back(Robo(id, true));
        robosAdv.push_back(Robo(id, false));
    }
}

GerenteVisao::~GerenteVisao()
{
}

void GerenteVisao::setIdRoboEq(int numRobo, int id){
    if(numRobo < 0 || numRobo > NUM_MAX_ROBOS){
        cout << "ERRO: Robo inválido" << endl;
    }

    robosEq[numRobo].setId(id);
}

void GerenteVisao::setIdRoboAdv(int numRobo, int id){
    if(numRobo < 0 || numRobo > NUM_MAX_ROBOS){
        cout << "ERRO: Robo inválido" << endl;
    }

    robosAdv[numRobo].setId(id);
}

void GerenteVisao::resetRobosSSL()
{
    for(unsigned i = 0 ; i < robosSSL.size();i++)
    {
        robosSSL[i] = RoboSSL();

    }
    for(unsigned i = 0 ; i < robosAdvSSL.size();i++)
    {
        robosAdvSSL[i] = RoboSSL();
    }
}

void GerenteVisao::atualizaRobosSSL()
{
    /// atualizando os robos encontrados, os que não foram encontrados e atualizando as posicoes deles
    for(int id=0;id<NUM_MAX_ROBOS;id++){

        /// se o robo foi encontrado iremos setar o robo no vetor de robo
        if(robosSSL[id].achado){

            /// setando os dados do robo
            robosEq[id].setPosicao(robosSSL[id].x, robosSSL[id].y, robosSSL[id].orientation);
            robosEq[id].setConfiabilidade(robosSSL[id].confidence);
            robosEq[id].incrementaContadorAchado();
        }else{
            /// caso o robo não tenha sido encontrado iremos incrementar o contador que ele não foi encontrado
            robosEq[id].incrementaContadorNaoAchado();
        }

        /// se o robo foi encontrado iremos setar o robo no vetor de robo
        if(robosAdvSSL[id].achado){

            /// setando os dados do robo
            robosAdv[id].setPosicao(robosAdvSSL[id].x, robosAdvSSL[id].y, robosAdvSSL[id].orientation);
            robosAdv[id].setConfiabilidade(robosAdvSSL[id].confidence);
            robosAdv[id].incrementaContadorAchado();

        }else{
            /// caso o robo não tenha sido encontrado iremos incrementar o contador que ele não foi encontrado
            robosAdv[id].incrementaContadorNaoAchado();
        }
    }
}

bool GerenteVisao::leuTodoCampo()
{
    foreach (char camera, camerasLidas)
    {
        if(!camera) return false;
    }
    return true;
}

void GerenteVisao::processPacket(const SSL_DetectionFrame &frameCam)
{
    if(frameCam.camera_id() < NUM_CAMERAS)
    {
        //cout << "Lendo camera " << frameCam.camera_id() << endl;
        camerasLidas[frameCam.camera_id()] = true;
    }else
    {
        cout << "ID da camera vindo da SSL invalido!!!!!!!!!" << endl;
    }

    //*******************************************************************/
    SSL_DetectionBall bolaAux;
    bola.setConfiabilidade(0);

    //! Descobre qual bola possui a maior confiabilidade
    int numBolas = frameCam.balls_size();
    for(int i=0;i<numBolas;i++){
        bolaAux = frameCam.balls(i);
        if(bola.getConfiabilidade()<bolaAux.confidence())
        {
//            cout << "encontrado bola no frame " << frameCam.camera_id() << endl;
            bola.setPosicao(bolaAux.x(),bolaAux.y());
            bola.setZ(bolaAux.z());
            bola.setConfiabilidade(bolaAux.confidence());
        }
    }

    //*******************************************************************

    //************************* Processando pacotes dos robos da equipe e do adversario ******************************************/
    SSL_DetectionRobot roboAux;
    unsigned int id;

    int numRobosAzuis=frameCam.robots_blue_size(), numRobosAmarelos=frameCam.robots_yellow_size();

    /// verificando qual é a cor do nosso time para selecionar o pacote certo
    if(config->corEquipe == Azul){

        /// analisando os robos da equipe que sao azuis
        for(int i=0; i < numRobosAzuis; i++){

            /// pegando o robo da equipe do frame dos robos azui enviado
            roboAux = frameCam.robots_blue(i);
            id = roboAux.robot_id();

            /// caso não tenha nenhum robo ainda iremos adicionar ele, caso contrário
            /// irá ser analisado a confiabilidade e irá ser pego o robo com maior confiabilidade
            /// @todo pegar o robo mais próximo da posição anterior dele
            if(!robosSSL[id].achado || robosSSL[id].confidence < roboAux.confidence())
            {
                //cout << "SSL achou robo equipe " << id << endl;
                robosSSL[id].achado = true;
                robosSSL[id].x = roboAux.x();
                robosSSL[id].y = roboAux.y();
                robosSSL[id].orientation = roboAux.orientation();
                robosSSL[id].confidence = roboAux.confidence();
            }
        }

        /// analisando os robos adversarios que amarelos
        for(int i=0; i < numRobosAmarelos; i++){

            /// pegando o robo adv do frame dos robos amarelos enviado
            roboAux = frameCam.robots_yellow(i);
            id = roboAux.robot_id();

            /// caso não tenha nenhum robo ainda iremos adicionar ele, caso contrário
            /// irá ser analisado a confiabilidade e irá ser pego o robo com maior confiabilidade
            /// @todo pegar o robo mais próximo da posição anterior dele
            if(!robosAdvSSL[id].achado || robosAdvSSL[id].confidence < roboAux.confidence()){
                //cout << "SSL achou robo adv " << id << endl;
                robosAdvSSL[id].achado = true;
                robosAdvSSL[id].x = roboAux.x();
                robosAdvSSL[id].y = roboAux.y();
                robosAdvSSL[id].orientation = roboAux.orientation();
                robosAdvSSL[id].confidence = roboAux.confidence();
            }
        }

    }else{

        /// analisando os robos da equipe que são os amarelos
        for(int i=0; i < numRobosAmarelos; i++){

            /// pegando o robo da equipe do frame dos robos azui enviado
            roboAux = frameCam.robots_yellow(i);
            id = roboAux.robot_id();

            /// caso não tenha nenhum robo ainda iremos adicionar ele, caso contrário
            /// irá ser analisado a confiabilidade e irá ser pego o robo com maior confiabilidade
            /// @todo pegar o robo mais próximo da posição anterior dele
            if(!robosSSL[id].achado || robosSSL[id].confidence < roboAux.confidence())
            {
                //cout << "SSL achou robo equipe " << id << endl;

                robosSSL[id].achado = true;
                robosSSL[id].x = roboAux.x();
                robosSSL[id].y = roboAux.y();
                robosSSL[id].orientation = roboAux.orientation();
                robosSSL[id].confidence = roboAux.confidence();
            }
        }

        /// analisando os robos adversarios que são azuis
        for(int i=0; i < numRobosAzuis; i++)
        {

            /// pegando o robo adv do frame dos robos amarelos enviado
            roboAux = frameCam.robots_blue(i);
            id = roboAux.robot_id();

            /// caso não tenha nenhum robo ainda iremos adicionar ele, caso contrário
            /// irá ser analisado a confiabilidade e irá ser pego o robo com maior confiabilidade
            /// @todo pegar o robo mais próximo da posição anterior dele
            if(!robosAdvSSL[id].achado || robosAdvSSL[id].confidence < roboAux.confidence())
            {
                //cout << "SSL achou robo adv " << id << endl;
                robosAdvSSL[id].achado = true;
                robosAdvSSL[id].x = roboAux.x();
                robosAdvSSL[id].y = roboAux.y();
                robosAdvSSL[id].orientation = roboAux.orientation();
                robosAdvSSL[id].confidence = roboAux.confidence();
            }
        }
    }
    //atualizaRobosSSL();
}

PacoteVisao GerenteVisao::montaPacoteVisao(){

    PacoteVisao pctVisao;
    PacoteBola *pctBola;
    PacoteRobo *pctRobo;

    numFrameVisao++;
    pctVisao.set_numeroframe(numFrameVisao);
    pctVisao.set_delay(0.0);

    pctBola = pctVisao.mutable_bola();
    pctBola->set_x(bola.getPosicao()[0][0]);
    pctBola->set_y(bola.getPosicao()[1][0]);
    pctBola->set_z(bola.getZ());

    /// criando o pacote dos robos da equipe
    int contRobos=0;
    for(int id=0; id < NUM_MAX_ROBOS && contRobos < NUM_ROBOS_JOGANDO; id++){

        /// verificando se o robo foi achado, caso tenha sido iremos adicionar no pacote de dados
        if(robosEq[id].foiAchado()){

            /// setando os dados do robo
            pctRobo = pctVisao.add_robosequipe();
            pctRobo->set_id(id);
            pctRobo->set_x(robosEq[id].getPosicao()[0][0]);
            pctRobo->set_y(robosEq[id].getPosicao()[1][0]);
            pctRobo->set_orientation(robosEq[id].getPosicao()[2][0]);

            /// incrementando a quantidade de robos que iremos enviar, pois temos que analisar senão passa do maximo de robos possiveis
            /// @todo caso tenha mais robos do que o possivel para jogar, temos que pegar os robos mais confiaveis
            contRobos++;
        }
    }

    contRobos=0;
    /// criando o pacote dos robos adversarios
    for(int id=0; id < NUM_MAX_ROBOS && contRobos < NUM_ROBOS_JOGANDO;id++){

        /// verificando se o robo foi achado, caso tenha sido iremos adicionar no pacote de dados
        if(robosAdv[id].foiAchado()){

            /// setando os dados do robo
            pctRobo = pctVisao.add_robosadversarios();
            pctRobo->set_id(id);
            pctRobo->set_x(robosAdv[id].getPosicao()[0][0]);
            pctRobo->set_y(robosAdv[id].getPosicao()[1][0]);
            pctRobo->set_orientation(robosAdv[id].getPosicao()[2][0]);

            /// incrementando a quantidade de robos que iremos enviar, pois temos que analisar senão passa do maximo de robos possiveis
            /// @todo caso tenha mais robos do que o possivel para jogar, temos que pegar os robos mais confiaveis
            contRobos++;
        }
    }

    return pctVisao;
}
