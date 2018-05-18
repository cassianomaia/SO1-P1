# SO1-P1
Repositório para armazenar o primeiro trabalho da disciplina de Sistemas Operacionais 1.

### Tema Abordado

**5. Hilzer’s Barbershop problem**

Our barbershop has three chairs, three barbers, and a waiting area that can accommodate four customers on a sofa and that has standing room for additional customers. Fire codes limit the total number of customers in the shop to 20. A customer will not enter the shop if it is filled to capacity with other customers. Once inside, the customer takes a seat on the sofa or stands if the sofa is filled. When a barber is free, the customer that has been on the sofa the longest is served and, if there are any standing customers, the one that has been in the shop the longest takes a seat on the sofa. When a customer’s haircut is finished, any barber can accept payment, but because there is only one cash register, payment is accepted for one customer at a time. The barbers divide their time among cutting hair, accepting payment, and sleeping in their chair waiting for a customer.

### Instruções para compilação do programa

Para compilar e executar o programa realizamos os seguintes passos:

1 - Instale a biblioteca SFML: sudo apt-get install libsfml-dev
(para Debian e derivados, checar pagina do SFML para outras distros)

2 - Compile: g++ barbermain.c queue.c -o barber -lpthread -lm -lsfml-graphics -lsfml-window -lsfml-system

3 - Execute: ./barber param1 param2

**OBS:** 

- O param1 a ser passado é referente ao número de clientes da barbearia.
- O param2 a ser passado é um número aleatório para setar a seed de tempo utilizada.

### Realizado pelos alunos:
* Anderson Alan Montor - RA:726495
* Cassiano Maia - RA:726507
* Julia Milani  - RA:726552
* Mariana Cavichioli Silva - RA:726568

**Grupo 02.**

**Disciplina:** Sistemas Operacionais 01 - Profª Kellen Vivaldini
