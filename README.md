Library for easy connection PC to Board.
As of now it's header-only and supports only UART and only for Windows.
Final version should allow connection via UART, Bluetooth, WiFi and IP protocols. Linux support is planned as well.

Пример использования (Windows):
```
/* ....  */
#include <iostream>
#include <string>

/* ....  */

  //подключаем главный заголовочный файл:
  //
#include "include/BoardConnect.h" 

/* ....  */

int main(){

  //задаем параметры подключения по Uart.
  //для этого создаем экземпляр класса UartConnectionSettings
  //параметры задаются в его конструкторе. 
  //Значения по умолчанию: COM1, 9600бод, 8 бит, без проверки четности, один стоп-бит
  //для примера поменяем только номер порта и скорость (все остальные параметры останутся по умолчанию):
  //
board_connect::uart::UartConnectionSettings my_uart_settings{ board_connect::uart::COM2, 115200 };

  //создаем экземпляр класса Board. Инстанцируем типом std::string (это также тип данных по умолчанию)
  //это значит, что принимать и посылать данные мы будем с помощью данного типа.
  //Можно использовать свой тип (для этого придется перегрузить функцию board_connect::Data,
  //переводящую пользовательский формат в Си-строку и обратно)
  //Эта функция (она уже перегружена правильным образом для std::string) пока что находится в файле Declarations.h
  //
board_connect::Board<std::string> my_board = board_connect::MakeUartBoard<std::string>(my_uart_settings);

  //подключаемся к выбранному порту:
  //
auto connection_attempt = my_board.Connect();

  //проверка:
  //
if(connection_attempt != board_connect::ConnectionStatus_t::CONNECTED_OK){
  //не получилось подключиться: нет указанного порта или другая причина. 
  //В данном примере просто прекратим выполнение.
  return 0;
}

  //посылаем сообщение на порт:
  //
bool send_result = my_board.Send("Hello, board!");

  //если Send() возвращает true, значит, сообщение передано в выводной буфер 
  //оно будет переведено в Си-строку и отправлено в соответствующем потоке (SenderLoop).
  //если Send() возвращает false, то ошибка при сохранении в буфер.
  //пока просто закончим выполнение в таком случае.
  //
if(!send_result){
  my_board.Disconnect();
  return 0;
}

/* .... */

  //Допустим, прошло какое-то время и мы хотим проверить, пришло ли что-то из порта.
  //метод Receive() возвращает std::optional с нашим типом данных внутри, если получено что-то
  //Если приемный буфер пуст, будет возвращен std::nullopt
  //обновление входного буфера также выполняется в отдельном потоке (ReceiveBuffer)
  //
auto received_data = my_board.Receive();

  //выводим на экран, если приняли что-то
  //
if(received_data) std::cout << *received_data << std::endl;

/* ....  */

  //проверим статус подключения:
  //
auto status = my_board.Status();

if(status == board_connect::ConnectionStatus_t::CONNECTED_OK){
  std::cout << "Connection still OK"<< std::endl;
}
else {
  std::cout << "Connection lost!" <<endl;
}

/* ....  */
  //отключаемся
  //
my_board.Disconnect();

return 0;
}  //end main
```

