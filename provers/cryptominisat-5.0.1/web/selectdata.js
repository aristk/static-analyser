var columnDivs = [];
var graph_data = [];
var clDistrib = [];
var simplificationPoints = [];
var maxConflRestart = [];

//while (true) {
//setInterval(function(){myajax.makeGetRequest(500005960);}, 2000);
//myajax.makeGetRequest(86533651);
//}

function selected_runID(runID) {
    console.log("getting runid " + runID);
    clear_everything();
    link = "getdata.php?id=" + runID;
    //runID = "/private/sat/dat/" + runID.replace(/^.*[\\\/]/, '') + ".dat"
    //link = runID;
    jQuery.getJSON(
        link
        , function(response)
         {
            console.log("parsing data");
            var v = document.getElementById("fileinfo");
            v.innerHTML = "Status: ";
            var metad = response["metadata"];
            if (metad["status"] == null) {
                v.innerHTML += "Unfinished";
            } else {
                v.innerHTML += metad["status"];
                v.innerHTML += ", time(s): "+ metad["difftime"];
                //v.innerHTML += ", start time: "+ metad["startTime"];
                //v.innerHTML += ", end time: "+ metad["endTime"];
            }
            v.innerHTML += ", run id: "+runID;

            columnDivs = response["columnDivs"];
            graph_data = response["graph_data"];
            clDistrib = [];
            simplificationPoints = response["simplificationPoints"];
            maxConflRestart = response["maxConflRestart"];
            print_all_graphs();
        }
    );
}

//selected_runID(15772794);
