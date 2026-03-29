package main

import (
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"github.com/gorilla/websocket"
)

const (
	P2PPort = 8080
	WebPort = 8081
)

var (
	upgrader = websocket.Upgrader{
		CheckOrigin: func(r *http.Request) bool { return true },
	}
	uiChan = make(chan string, 100)
)

func main() {
	if len(os.Args) != 2 {
		fmt.Printf("Usage: go run main.go <Target IP>\n")
		os.Exit(1)
	}
	targetIP := os.Args[1]

	addr, _ := net.ResolveUDPAddr("udp", fmt.Sprintf(":%d", P2PPort))
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		log.Fatalf("UDP Error: %v\n", err)
	}
	defer conn.Close()

	targetAddr, _ := net.ResolveUDPAddr("udp", fmt.Sprintf("%s:%d", targetIP, P2PPort))

	fmt.Printf("=== Go P2P Chat Started ===\n")
	fmt.Printf("Target IP: %s\n", targetIP)

	//UDP受信ループ
	go func() {
		buffer := make([]byte, 1024)
		for {
			n, remoteAddr, err := conn.ReadFromUDP(buffer)
			if err != nil {
				continue
			}
			msg := string(buffer[:n])
			log.Printf("[UDP受信] %s から受信: %s\n", remoteAddr.IP, msg)

			select {
			case uiChan <- msg:
				log.Println("[内部処理] ブラウザ転送用にチャネルへ格納成功")
			default:
				log.Println("[エラー] ブラウザ未接続のため破棄:", msg)
			}
		}
	}()

	http.Handle("/", http.FileServer(http.Dir(".")))

	http.HandleFunc("/ws", func(w http.ResponseWriter, r *http.Request) {
		ws, err := upgrader.Upgrade(w, r, nil)
		if err != nil {
			log.Println("WebSocket Upgrade Error:", err)
			return
		}
		defer ws.Close()
		log.Println("[WebSocket] ブラウザが接続しました")

		//Go -> ブラウザ
		go func() {
			for msg := range uiChan {
				err := ws.WriteMessage(websocket.TextMessage, []byte(msg))
				if err != nil {
					log.Println("[WebSocket切断] ブラウザへの送信停止")
					break
				}
				log.Println("[WS送信] ブラウザへ表示指示を出しました")
			}
		}()

		//ブラウザ -> Go -> UDP
		for {
			_, message, err := ws.ReadMessage()
			if err != nil {
				break
			}
			log.Printf("[WS受信] ブラウザから入力あり: %s\n", string(message))

			_, err = conn.WriteToUDP(message, targetAddr)
			if err != nil {
				log.Println("[UDP送信エラー]:", err)
			} else {
				log.Printf("[UDP送信] %s へパケット送信完了\n", targetAddr.String())
			}
		}
	})

	log.Fatal(http.ListenAndServe(fmt.Sprintf(":%d", WebPort), nil))
}