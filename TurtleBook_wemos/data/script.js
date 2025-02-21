    
    window.addEventListener("DOMContentLoaded", function() {

    	var toggler = document.getElementsByClassName("caret");
var i;

for (i = 0; i < toggler.length; i++) {
  toggler[i].addEventListener("click", function() {
    this.parentElement.querySelector(".nested").classList.toggle("active");
    this.classList.toggle("caret-down");
  });
} 

    console.log(document.getElementById("sub1"));

			document.getElementById("sub1").addEventListener("click",function(e){
				e.preventDefault();
				e.stopPropagation();
				e.stopImmediatePropagation();
				//uploadZip(e);
				uploadZipHuff(e);

				return false;
			});

			async function uploadZip(event){

					var data=new FormData();
					 var input=document.querySelector('input[type="file"]');
					 var upn = document.querySelector(" input[name='uploadPathInput']");
					 console.log('upn',upn);
					 console.log('upn.value',upn.value);
					 var zip=new JSZip();
					 zip.file(input.files[0].name,input.files[0]);
					 var blob=await zip.generateAsync({type: "blob",
					 	compression:"DEFLATE"
					});

					 data.append('file',blob,'upload.zip');
					 data.append('uploadPathInput',upn.value);
					 const response = await fetch('/upload',{
					 	method: 'POST',
					 	body: data
					 });
					 if(response.status==200){
					 	window.location.reload();
					 }else{
					 	console.log('error');
					 }
			}
    }, false);
