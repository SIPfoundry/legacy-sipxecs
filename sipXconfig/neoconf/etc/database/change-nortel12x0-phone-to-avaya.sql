-- updates the registered nortel 12x0 phones model's name from "nortel-12x0" to "avaya-12x0"
update phone set model_id='avaya-1210' where model_id='nortel-1210';
update phone set model_id='avaya-1220' where model_id='nortel-1220';
update phone set model_id='avaya-1230' where model_id='nortel-1230';