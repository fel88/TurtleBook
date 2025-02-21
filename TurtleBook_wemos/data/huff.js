// Online Javascript Editor for free
// Write, Edit and Run your Javascript code using JS Online Compiler

console.log("Welcome to Programiz!");


class Node {
  constructor(char, frequency) {
    this.char = char;
    this.frequency = frequency;
    this.left = null;
    this.right = null;
  }
}

 function buildHuffmanTree(frequencyTable) {
                let nodes = [];
                for (let char in frequencyTable) {
                    nodes.push(new Node(char, frequencyTable[char]));
                    hnodes[char] = nodes[nodes.length - 1];
                }


                while (nodes.length > 1) {
                    nodes.sort((a, b) => a.frequency - b.frequency);
                    let left = nodes.shift();
                    let right = nodes.shift();
                    let parent = new Node(null, left.frequency + right.frequency);
                    left.parent = parent;
                    right.parent = parent;
                    parent.left = left;
                    parent.right = right;
                    nodes.push(parent);
                }
                console.log(nodes);
                console.log('hnodes', hnodes);
                return nodes[0];
            }
  let hnodes = {}

            let bits_cache = {};

            function getBits(b) {

                if (b in bits_cache) {
                    return bits_cache[b];
                }

                let lbits = []
                let node = hnodes[b];
                // console.log('node', node);
                while (node && node.parent) {
                    if (node.parent.left == node) {
                        lbits.push(0);
                    }
                    else {
                        lbits.push(1);
                    }
                    node = node.parent;
                }
                lbits.reverse();

                bits_cache[b] = lbits;
                return lbits;
            }


function encode_my(bts) {
                let bits = []
                let output = []
                //  console.log('bts', bts);
                for (let ii = 0; ii < bts.length; ii++) {
                    //if ((ii % 1000)==0)
                    // console.log('byte', ii, ' / ', bts.length);
                    let b = bts[ii];
                    let lbits=getBits(b);                    
                    bits = bits.concat(lbits);
                    
                    while (bits.length >= 8) {
                        let bb = 0;
                        for (let j = 0; j < 8; j++) {
                            bb |= ((bits[j]) << j);
                        }
                        output.push(bb);
                        bits = bits.slice(8);
                    }
                }
                //alert('cp2');
                // console.log('cp2');
                while (bits.length % 8 != 0) {
                    bits.push(0);
                }
                //  alert('cp3');
                //console.log('gbits', bits)
                for (let i = 0; i < bits.length; i += 8) {
                    let b = 0;
                    for (let j = 0; j < 8; j++) {
                        b |= ((bits[i + j]) << j);
                    }
                    output.push(b);
                }
                // console.log('cp3');
                return output;
            }



function buildFrequencyTable(string) {
  let frequencyTable = {};
  for (let char of string) {
    frequencyTable[char] = frequencyTable[char] + 1 || 1;
  }
  return frequencyTable;
}   

longToByteArray = function (/*long*/long) {
                // we want to represent the input as a 8-bytes array
                var byteArray = [0, 0, 0, 0, 0, 0, 0, 0];

                for (var index = 0; index < byteArray.length; index++) {
                    var byte = long & 0xff;
                    byteArray[index] = byte;
                    long = (long - byte) / 256;
                }

                return byteArray;
            };
            function string2Bin(str) {
                var result = [];
                for (var i = 0; i < str.length; i++) {
                    result.push(str.charCodeAt(i));
                }
                return result;
            }

function decode_my(bts)
        {
            let output = []
            let p=root;
            for (let ii=0;ii< bts.length;ii++)
            {
                let item=bts[ii];
                let bits = []
                for (let i = 0; i < 8; i++)
                {
                    if ((item & (1 << i)) != 0) bits.push(1);
                    else bits.push(0);
                }
                for (let kk=0;kk< bits.length;kk++)
                {
                    let bit=bits[kk];
                    if (p.char )
                    {
                        output.push(p.char);
                        p = root;
                    }
                    if (bit == 1)
                    {
                        p = p.right;
                    }
                    else
                    {
                        p = p.left;
                    }
                }
            }
            if (p.char)
            {
                output.push(p.char);
            }
            return output;
        }
        
 async function uploadZipHuff(event) {

                var data = new FormData();
                var input = document.querySelector('input[type="file"]');
     var upn = document.querySelector(" input[name='uploadPathInput']");
                     console.log('upn',upn);
                     console.log('upn.value',upn.value);
data.append('uploadPathInput',upn.value);

                //zip.file(input.files[0].name,input.files[0]);
                /*var blob=await zip.generateAsync({type: "blob",
                     compression:"DEFLATE"
               });*/

                 
                //data.append('file',blob,'upload.zip');
                var reader = new FileReader();
                var fileByteArray = [];
                reader.readAsArrayBuffer(input.files[0]);
                reader.onloadend = async function (evt) {
                    if (evt.target.readyState == FileReader.DONE) {
                        var arrayBuffer = evt.target.result,
                            array = new Uint8Array(arrayBuffer);
                        for (var i = 0; i < array.length; i++)
                            fileByteArray.push(array[i]);

                        //  fileByteArray=[0x01,0x02,0x02,0x03];
                        let ft = buildFrequencyTable(fileByteArray);
                        //fileByteArray = [0x01, 0x02, 0x02, 0x03, 0x01];
                        let root = buildHuffmanTree(ft);
                        let blob = encode_my(fileByteArray);
                        console.log(blob)
                        let vtable = []
                        for (let l = 0; l < 256; l++) {
                            vtable.push(l);
                            let ee = getBits(l);
                            console.log('getBits', l,ee)
                            vtable.push(ee.length);
                            vtable = vtable.concat(ee);
                        }

                        blob = string2Bin('ZCB').concat(longToByteArray(input.files[0].size)).concat(vtable).concat(blob);
                        //blob=new Blob(blob, { type: "text/html" }); // the blob
                        blob = new Blob([new Uint8Array(blob).buffer]); // the blob
                        console.log(blob)
                        data.append('file', blob, input.files[0].name + '.zcb');
                        const response = await fetch('/upload', {
                            method: 'POST',
                            body: data
                        });
                        if (response.status == 200) {
                            window.location.reload();
                        } else {
                            console.log('error');
                        }

                    }
                }


            }
