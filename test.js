var cache = require("./js0n-cache");
var fs = require("fs");

var doSubKeyTest = false;

var data = fs.readFileSync("medium.json").toString("utf8");


var startTime = Date.now();
for (var i = 0; i < 1000; ++i) {
    var o = JSON.parse(data);
    if (o === undefined) {
        console.log("ZOMG");
    }
    if (doSubKeyTest && o["web-app"]["servlet-mapping"]["fileServlet"] != "/static/*") {
        console.log("ERROR");
    }
}
console.log("JSON took " + (Date.now() - startTime) + " to run 1000 times");


var startTime = Date.now();
for (var i = 0; i < 1000; ++i) {
    var o = cache.parse(data);
    if (o === undefined) {
        console.log("ZOMG");
    }
    if (doSubKeyTest && o["web-app"]["servlet-mapping"]["fileServlet"] != "/static/*") {
        console.log("ERROR");
    }
}
console.log("js0n took " + (Date.now() - startTime) + " to run 1000 times");




/*

var jsonStr = '{"test":"strings", "numVal":1234, "someBool":true, "yuck":null, "subObj":{"subValue":1, "array":[1, 2, 3, 4, 5, 6, 5,5,5,5,5,5,5,5], "lastKey":"wrong"}, "lastKey":"last!"}';
console.log("Json length: " + jsonStr.length);
var o = cache.parse(jsonStr);

function checkKey(key) {
    var val = o[key];
    console.log("-----------------------------------------------------------------------------------");
    console.log("  Key: " + key);
    console.log("Value: " + val);
    console.log(" Type: " + typeof(val));
    console.log("===================================================================================");
}

checkKey("someBool");
checkKey("test");
checkKey("numVal");
checkKey("yuck");
console.log("subValue should be 1: " + o.subObj.subValue);
checkKey("lastKey");
*/
