var SS = SpreadsheetApp.openById(
  '1Y2nbVf79pYpHzzZb1esRbfI0kS8q2igLRaS0Wg-nyn4'
); // Enter Your Sheet ID Got From Sheet URL Link

function doPost(data) {
  var parsedData;
  var response = '';
  var currentDate = new Date();

  try {
    parsedData = JSON.parse(data.postData.contents);
  } catch (e) {
    return ContentService.createTextOutput(
      'Error in parsing request body: ' + e.message
    );
  }

  if (parsedData === undefined)
    return ContentService.createTextOutput(
      'Error! Request body empty or in incorrect format.'
    );

  switch (parsedData.command) {
    case 'appendRow':
      try {
        var decimalData = parsedData.values.toString().replaceAll('.', ',');
        var dataArr = decimalData.split(';');
        dataArr.unshift(currentDate);

        var sheet = SS.getSheetByName(getSheetNameByDate(currentDate));
        if (!sheet) sheet = createDefaultSheet();

        sheet.appendRow(dataArr);

        response = 'Success';
      } catch (e) {
        response = 'Error\n' + JSON.stringify(parsedData) + '\n' + e;
      }
      SpreadsheetApp.flush();
      break;
  }
  return ContentService.createTextOutput(response);
}

function getSheetNameByDate(date) {
  var month = date.getMonth() + 1;
  var year = date.getFullYear();
  return month.toString() + '.' + year.toString();
}

function createDefaultSheet() {
  var sheetName = getSheetNameByDate(new Date());
  var sheet = SS.getSheetByName(sheetName);
  if (!sheet) sheet = SS.insertSheet(sheetName, 0);

  sheet.appendRow([
    'Date',
    'Wattering required ON/OFF',
    'measurement',
    'average measurement',
    'manual watering',
  ]);

  return sheet;
}
