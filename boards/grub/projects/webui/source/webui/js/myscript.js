<!-- Set of scripts -->

function XHRReadyHandle(request, handle)
{
	if(request.readyState == 4 && request.status == 200) 
	{
		/* handle(request.responseXML);  -- for XML */
		handle(request.responseText); /* -- for JSON */
	}
}

function XHRGet(handle, url, async) 
{
	var request = null;
	var message = "XHRGet(" + url + ")";

	console.log(message);
	
	request = new XMLHttpRequest();
	if (request == null) {
		alert("Your browser does not support XMLHttp");
		return -1;
	}
	
	if (handle != null)
		request.onreadystatechange = function () { XHRReadyHandle(request, handle); };
	
	request.open("GET", url, async);

	request.send(null); /* null content --> content_length = 0 */
	return 0;
}

function XHRPost(handle, url, param, async)
{
	var request = null;
	var message = "XHRPost(" + url + ")";

	console.log(message);
	
	request = new XMLHttpRequest();
	if (request == null) {
		alert("Your browser does not support XMLHttp");
		return -1;
	}

	request.open("PUT", url, async);
	request.setRequestHeader("Content-type", "application/json");
	if (handle != null)
		request.onreadystatechange = function () { XHRReadyHandle(request, handle); };

	request.send(param); /* content... */
	return 0;
}

function XHRUpload(handle, url, file)
{
	var message = "XHRUpload(" + url + ")";
	console.log(message);
	
	var request = new XMLHttpRequest();
	if (request == null) {
		alert("Your browser does not support XMLHttp");
		return -1;
	}

	var reader = new FileReader();
	if (reader == null) {
		alert("Your browser does not supprt FileReader");
		return -1;
	}

	request.upload.addEventListener("progress", function(e) {
		if (e.lengthComputable) {
			const percentage = Math.round( (e.loaded * 100) / e.total );
			/* ctrl.update(percentage); */
			console.log("Percentage="+percentage);
		}}, false);

	request.upload.addEventListener("load", function(e) {
		/* ctrl.update(100); */
		console.log("Load event called....");
		}, false);

	request.open("POST", url);
	/* request.overrideMimeType("application/octet-stream"); */
	request.overrideMimeType("text/plain; charset=UTF-8");
	reader.onload = function (evt) {
		request.send(evt.currentTarget.result);
		console.log("Reader::OnLoad event !!");
	};
	reader.readAsBinaryString(file);
	return 0;
}

