const ws = new WebSocket(`ws://${window.location.host}/ws`);
const messageArea = document.getElementById('messageArea');
const input = document.getElementById('messageInput');
const sendBtn = document.getElementById('sendBtn');
const targetId = document.getElementById('targetId');
const stampToggleBtn = document.getElementById('stampToggleBtn');
const stampPalette = document.getElementById('stampPalette');
const stampMap = {
    'happy': 'stamps/happy.png',
    'ok': 'stamps/OK.png',  
    'sad': 'stamps/sad.png' ,
    'HBD': 'stamps/HBD.png'
};

function initStampPalette() {
    for (const [id, url] of Object.entries(stampMap)) {
        const img = document.createElement('img');
        img.src = url;
        img.alt = id;
        img.onclick = function() {
            sendStamp(id);
            stampPalette.classList.add('hidden');
        };
        stampPalette.appendChild(img);
    }
}
initStampPalette();

stampToggleBtn.onclick = function() {
    stampPalette.classList.toggle('hidden');
};

ws.onopen = function() {
    console.log("[WS] サーバーとの接続が確立しました");
};

ws.onmessage = function(event) {
    const data = event.data;

    if (typeof data === 'string' && data.startsWith("INIT:")) {
        const ip = data.replace("INIT:", "");
        targetId.textContent = ip;
        return; 
    }

    try {
        const receivedMessage = JSON.parse(data);
        if (receivedMessage.type === 'stamp') {
            addMessage(receivedMessage.stampId, 'other', 'stamp');
        } else if (receivedMessage.type === 'text') {
            addMessage(receivedMessage.content, 'other', 'text');
        }
    } catch (error) {
        addMessage(data, 'other', 'text');
    }
};

sendBtn.onclick = sendTextMessage;
input.onkeypress = function(event) {
    if (event.key === 'Enter') {
        sendTextMessage();
    }
};
function sendTextMessage() {
    const text = input.value.trim();
    if (text === '') return;
    if (text.startsWith('@')) {
        const stampId = text.substring(1);
        if (stampMap[stampId]) {
            sendStamp(stampId);
            input.value = '';
            return;
        }
    }

    console.log("[WS送信] テキストを送信します:", text);
    const messageToSend = { type: 'text', content: text };
    addMessage(text, 'mine', 'text');
    ws.send(JSON.stringify(messageToSend)); 
    input.value = '';
}

function sendStamp(stampId) {
    console.log("[WS送信] スタンプを送信します:", stampId);
    const messageToSend = { type: 'stamp', stampId: stampId };
    addMessage(stampId, 'mine', 'stamp');
    ws.send(JSON.stringify(messageToSend)); 
}

function addMessage(content, type, contentType) {
    const wrapper = document.createElement('div');
    wrapper.className = `message-wrapper ${type}`;
    
    const bubble = document.createElement('div');
    bubble.className = 'message-bubble';
    
    if (contentType === 'stamp') {
        const stampImg = document.createElement('img');
        const stampUrl = stampMap[content];
        if (stampUrl) {
            stampImg.src = stampUrl;
            bubble.classList.add('stamp-bubble'); 
            bubble.appendChild(stampImg);
        }
    } else {
        bubble.textContent = content;
    }
    
    wrapper.appendChild(bubble);
    messageArea.appendChild(wrapper);
    messageArea.scrollTop = messageArea.scrollHeight; 
}