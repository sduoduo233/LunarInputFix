import frida

session = frida.attach("javaw.exe")
script = session.create_script("""
function readMessage(p){
    var hwnd = p.readPointer()
    var message = p.add(Process.pointerSize).readUInt()
    var wParam = p.add(Process.pointerSize+8).readU64()
    var lParam = p.add(Process.pointerSize+8+8).readU64()
    return {
        "hwnd": hwnd,
        "message": message,
        "wParam": wParam,
        "lParam": lParam
    }
}

var msgPointer
var messages = []
Interceptor.attach(Module.getExportByName('user32.dll', 'PeekMessageW'), {
    onEnter(args) {
        msgPointer = args[0];
    },
    onLeave(retval) {
        
        if(retval == 0x0){
            if(messages.length > 0){
                var msg = messages.shift()
                msgPointer.writePointer(msg.hwnd)
                msgPointer.add(Process.pointerSize).writeUInt(msg.message)
                msgPointer.add(Process.pointerSize+8).writeU64(msg.wParam)
                msgPointer.add(Process.pointerSize+8+8).writeU64(msg.lParam)
                console.log("--> SEND", JSON.stringify(readMessage(msgPointer)))
                retval.replace(0x1)
            }
            return
        }
        
        var msg = readMessage(msgPointer)
        
        // CHAR
        if(msg.message == 258){
            var char = msg.wParam & 65535
            console.log("char", JSON.stringify(msg), String.fromCharCode(char))
            if(char > 255){
                retval.replace(0x0)
                
                // keydown VK_PACKET
                messages.push({
                    "hwnd": msg.hwnd,
                    "message": 0x0100,
                    "wParam": 0xE7,
                    "lParam": 0x1
                })
                // CHAR
                messages.push({
                    "hwnd": msg.hwnd,
                    "message": 0x0102,
                    "wParam": char,
                    "lParam": 0x1
                })
                // keyup VK_PACKET
                messages.push({
                    "hwnd": msg.hwnd,
                    "message": 0x0101,
                    "wParam": 0xE7,
                    "lParam": 0xC0000001
                })
            }
        }
        
        // KEYDOWN
        if(msg.message == 256){
            var keycode = msg.wParam
            var pervious_state = (msg.lParam >>> 30) & 0x1
            var state = 1 - ((msg.lParam >>> 31) & 0x1)
            console.log("keydown", JSON.stringify(msg), String.fromCharCode(keycode))
        }
    }
})
""")

def on_message(message, data):
    print(message)
script.on('message', on_message)
script.load()
input()