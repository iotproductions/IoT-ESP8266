/*
 Rebuild by Thanh Trieu
 Date: 21h 02/10/2014
 Usage
0. Create a Spreadsheet and add A1 cell lable "Timestamp", change sheet name from Sheet1 to DATA and some Title(No limit)
1. Run the "setup" function (you'll need to do this twice - 1st time to grant acces to Script Properties)
2. Publish > Deploy as web app ... set security level and enable service
    Execute the app as: me(yourmail@gmail.com)
    Who has access to the app: Anyone, event anomynous
3. Copy the service URL and post this in your form/script action  
4. Insert column names on the "DATA" named sheet matching the parameter names of the data you are passing
*/
  
function doGet(e) { // change to doPost(e) if you are recieving POST data
  var ss = SpreadsheetApp.openByUrl('https://docs.google.com/spreadsheets/d/1ZDbbt8tHB8dbzxYHqPyTp5x4JBhwdEekB1yqs9CSeug/edit#gid=0');
  var sheet = ss.getSheetByName("DATA");//Sheet's name
  var headers = sheet.getRange(1, 1, 1, sheet.getLastColumn()).getValues()[0]; //read headers
  var nextRow = sheet.getLastRow(); // get next row
  var cell = sheet.getRange('a1');
  var col = 0;
  for (i in headers)
  { // loop through the headers and if a parameter name matches the header name insert the value
    if (headers[i] == "Timestamp")
    {
      val = new Date();
    } else 
    {
      val = e.parameter[headers[i]]; 
    }
    cell.offset(nextRow, col).setValue(val);
    col++;
  }
  //http://www.google.com/support/forum/p/apps-script/thread?tid=04d9d3d4922b8bfb&hl=en
  var app = UiApp.createApplication(); // included this part for debugging so you can see what data is coming in
  var panel = app.createVerticalPanel();
  for( p in e.parameters){
    panel.add(app.createLabel(p +" "+e.parameters[p]));
  }
  app.add(panel);
  return app;
   
}
//http://www.google.sc/support/forum/p/apps-script/thread?tid=345591f349a25cb4&hl=en
function setUp() {
  ScriptProperties.setProperty('active', SpreadsheetApp.getActiveSpreadsheet().getId());
}
