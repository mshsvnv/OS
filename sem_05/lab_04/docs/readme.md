демон момент

Терминал
--
запустить демона: 

`sudo ./a.out`
 
показать, что он работает:

`ps -ajx`

узнать id из файла блокировки:

`cat /var/run/daemon.pid`

посмотреть лог:

`sudo cat /var/log/syslog | grep dasha_deamon` (за такое название демона хихикали)

вызывать SIGHUB:

`sudo kill -1 PID`

вызывать SIGTERM:

`sudo kill -15 PID` 

убить демона:

`sudo kill -9 PID`


Вопросы:
-- 

- что делает твой демон?

  с периодичностью в SLEEP_TIME пишет в лог файл дату и время

- в какой момент процесс становится демоном?

  показать на вызов функции daemonize() в мейне

- зачем делать umask? где делать umask?

  чтобы сбросить маску режима создания файлов, вызывается в предке. таким образом процесс может создавать файлы с любыми правами доступа

- пояснить за каждую колоночку

  | PPID | PID | PGID | SID | TTY | TPGID | STAT |
  |------|-----|------|-----|------|------|------|
  | 2059 | 19893 | 19893 |   19893 | ? | -1 | Ssl | 

  - почему PPID такой? почему его усынловляет 2059? 

    потому что демона запускаем сами из терминала и его усыновляет процесс -- потомок процесса с идентификатором 1

  - PID == PGID == SID ?
  
    демон является лидером группы и лидером сессии, а вообще он там единственный, но это мы говорим для себя!

  - TTY == ? 
  
    нет управляющего терминала

  - TPGID == -1 
  
    отсутсвует терминальная группа (т.к. нет управляющего терминала)

  - “S” - ожидание завершения события (interruptable sleep). 
  
    прерываемый сон (S) -- в состоянии ожидания завершения *события* (прерывать можем с помощью сигналов)
  
    непрерываемый сон (D) -- в состоянии ожидания завершения *процесса ввода-вывода*

  - “s” - лидер сессии
  - “l” - многопоточость
  
- что творится в функции lockfile?
   
  ```
  заполняем поля структуры flock
  struct flock fl;
   
  fl.l_type = F_WRLCK; - блокируем разделение на запись
  
  следующие строчки - блокировка всего файла от начала до конца
  
  fl.l_start = 0;
  fl.l_whence = SEEK_SET;
  fl.l_len = 0;
  
  передаем в fctln, то есть применяем конфигурацию блокировки к дескриптору

  return fcntl(fd, F_SETLK, &fl);
  ```
  

- для чего функция already_running?
  
  чтобы сохранить демона в единственной экземпляре

- с помощью какого механизма мы это реализуем?

  с помощью механизма блокировки файла, куда демон пишет свой id
  
  
- fd0, fd1, fd2 - что за файлы? 

  stdin, stdout, stderr

- setsid() - создает новый сеанс, в котором вызывающий процесс становится лидером группы процессов без управляющего терминала. Идентификатор группы процессов вызывающего процесса устанавливается равным идентификатору вызывающего процесса

- sigaction() -- используется для изменения выполняемого процессом действия при получении определённого сигнала

- SIGHUP -- это сигнал, отправляемый процессу, когда его управляющий терминал закрыт.
  
- sigemptyset -- инициализирует набор сигналов, указанный в set, и "очищает" его от всех сигналов.
  
С точки зрения ядра -- **терминальная сессия** -- это группа процессов, имеющих один идентификатор сеанса sid.

**Группа процессов** -- инструмент для доставки сигнала нескольким процессам, а также способ арбитража при доступе к терминалу. Идентификатор группы pgid равен pid создавшего её процесса -- лидера группы. 

**Сеанс** -- средство для контроля путем посылки сигналов над несколькими группами процессов со стороны терминального драйвера. Идентификатор сеанса sid равняется идентификатору pid, создавшего его процесса -- **лидера сеанса**. Одновременно с сеансом создаётся новая группа с pgid равным pid лидера сеанса. Поскольку переход группы из сеанса в сеанс невозможен, то создающий сеанс процесс не может быть лидером группы.