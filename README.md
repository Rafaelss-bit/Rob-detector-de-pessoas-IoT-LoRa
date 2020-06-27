# Robô detector de possíveis vítimas em uma ambiente de desastres 

# Descrição
  Robô desenvolvido em parceria da UFU com a Defesa Civil de Patos de Minas para o primeiro workshop de reduão de riscos de desastres do Alto Paranaíba e Nororeste Mineiro.
  O projeto consiste em um robô móvel capaz de detectar presença de pessoas e enviar a localização da mesma em tempo real para um servidor web. 
  
 # Materiais Utilizados e Desenvolvimento
   Foi utilizado um robo com rodas capaz de se locomover no ambiente de forma autônoma, para tal funcionalidade foi utilizado tres sensores ultrassônicos HC-SR04 no protótipo. 
 Para a detecção de presença de pessoas foi utilizado o sensor de temperatura digital MLX90614. A trasmissão das informações da temperatura e localização foi feita via LoRa
 utilizando o módulo LoRa/GPS Shield e para receber os dados foi utilizado um LoRa gateway LG01-P. Foi utilizado o servidor de rede The Things Network pra receber estes dados 
 e depois os mesmos eram encaminhados para o servidor de aplicação Cayenne para que o usuário tivesse acesso a um mapa para ver o posicionamento do robô e um ícone no qual
 emitia um alerta quando era detectado a presença de uma pessoa.
 
 # Autores 
 
 Rafael Pereira - https://www.linkedin.com/in/rafael-pereira-73bba9148/
 Otávio Augusto  - https://github.com/OtaviodaCruz
 
   
