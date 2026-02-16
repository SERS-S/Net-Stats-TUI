# Net-Stats-TUI

Стандарт языка: C17

<!--
# Раздел
- Блоки раздела
+ Произвольный текст
? Откуда берем информацию
-->

Разделы статистики:

# Overall Statistics
- Total packages output (сколько всего пакетов исходит из linux узла)
- Total packages input (сколько всего пакетов приходит в linux узел)
- Total connection outputs (сколько всего исходящих соединений)
- Total connection inputs (сколько всего входящих соединений)
? Пока хз

# Interfaces
- Total active interfaces (сколько всего интерфейсов)
+ Interfaces list (разделы): [device] [type] [state] [connection] [output_bites] [input_bites] [MTU] [MAC_address]
? Пока хз

# Addresses & DHCP/DNS
- IPv4/IPv6 адреса на интерфейсах
- DNS servers, search domains (еще показать кто управляет DNS: resolv.conf vs systemd-resolved)
? /etc/resolv.conf и /run/systemd/resolve/resolv.conf (если systemd-resolved)

# ARP NET
- Таблица маршрутов (main + дополнительные), default route, metrics (показать соседей в локальной сети с информацией о arp маршрутизацией)

# Connection & Sockets
- LISTEN-порты (TCP/UDP), ESTABLISHED соединения
- Топ remote endpoints, топ local ports
? Пока хз

# Protocol stats (TCP/UDP/IP/ICMP health-метрики)
- TCP: RetransSegs, InSegs/OutSegs, EstabResets, ListenOverflows, etc
- IP: InDiscards, InDelivers, OutDiscards…
- ICMP: In/Out errors, unreachable, time exceeded…
? /proc/net/snmp & /proc/net/netstat

# Wi-Fi
- SSID/BSSID/RSSI/bitrate/MCS/quality (in %)
? Пока хз

# Network Profiles
- Просто парсим весь NetworkManager (nmcli connection show)
? Пока хз
