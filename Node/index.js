let express = require('express');
let app = express();
app.listen(3000, () => console.log('Server running on port 3000!'));

app.get('/:name', (req, res) => {
  res.send('Voltage Values ' + req.params.name + '\n');
});