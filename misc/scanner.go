// Scanner for generating workable ip:port pairs for IP cameras from DNS 
// name patterns
// (c) 2013 Victor Agababov (vagababov@gmail.com), Sergey Shekyan (shekyan@gmail.com).
package main

import (
  "flag"
  "fmt"
  "io/ioutil"
  "net"
  "strings"
  "time"
)

const (
  server = "Netwave IP Camera"
)

var (
  localhost = net.IPv4(127, 0, 0, 1)

  ports       = flag.String("ports", "80", "comma separated ports to scan")
  parsedPorts = strings.Split(*ports, ",")
  
  suffix      = flag.String("domain", ".localhost", "domain to scan, e.g. .example.com")
  connectionTimeout = flag.Duration("conn_timeout", 5*time.Second, "timeout duration")
  processTimeout    = flag.Duration("process_timeout", 5*time.Second, "read/write analyze duration")

  parallelizm = flag.Int("parallelizm", 25, "number of concurrent resolutions")
  limitChan   = make(chan int, *parallelizm)
)

func process(response string) bool { 
  t := strings.Split(response, "\r\n") 
  for _, i := range t { 
    if strings.HasPrefix(i, "Server:") && strings.Contains(i, "Camera") { 
      return true 
    } 
  } 
  return false 
}

func get_version(response string) string {

  body := strings.Split(response, "\r\n\r\n")
  t := strings.Split(body[1], "\n")
  for _, i := range t {
    if strings.HasPrefix(i, "var sys_ver=") {
      return strings.Split(i, "=")[1]
    }
  }
  return "undefined"
} 
 
func resolve(prefix string) { 
  full := prefix + *suffix 
  if addr, err := net.ResolveIPAddr("ip", full); err == nil { 
    if addr.IP.Equal(localhost) { 
      fmt.Printf("%v %v localhost\n", full, addr) 
    } else { 
      for _, p := range parsedPorts { 
        conn, err := net.DialTimeout("tcp", addr.String()+":"+p, *connectionTimeout) 
        if err == nil { 
                                        // Make sure the connection will be closed. 
          conn.SetDeadline(time.Now().Add(*processTimeout)) 
          _, err = conn.Write([]byte("GET /get_status.cgi HTTP/1.0\r\n\r\n")) 
          if err == nil { 
            result, err := ioutil.ReadAll(conn) 
            if err == nil { 
              response := string(result) 
              if process(response) { 
                fmt.Printf("%v %v success %s %s\n", full, addr, p, get_version(response)) 
              } else { 
                fmt.Printf("%v %v other %s\n", full, addr, p) 
              }
            } else {
              fmt.Printf("%v %v ping %s\n", full, addr, p)
            }
          } else {
                      fmt.Printf("%v %v ping %s\n", full, addr, p)
          }
                                        conn.Close()
        } else {
          fmt.Printf("%v %v failure %s\n", full, addr, p)
        }
      }
    }
  }
  <-limitChan
}

func main() {
  flag.Parse()
  parsedPorts = strings.Split(*ports, ",")
  s1, s2 := 'a', 'a'
  for {
    for {
      for i := 0; i < 10000; i++ {
        t := fmt.Sprintf("%c%c%04d", s1, s2, i)
        limitChan <- i
        go resolve(t)
      }
      s2++
      if s2 == 'z' {
        s2 = 'a'
        break
      }
    }
    s1++
    if s1 > 'z' {
      break
    }
  }
}
      
